// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

// initialising historyct variable to -1 initially
int historyct = -1;
// setting max historyct to 16
#define MAX_HISTORY_CT 16
// array to store history
char histtemp[MAX_HISTORY_CT][128];
// prototype for history_arrow function defined at last
void history_arrow(int);
// codes of esc, up arrow, down arrow and csi 
#define ESC 27    
#define DOWN_ARROW 66  
#define UP_ARROW 65
#define CSI 91  
int history_ind;
// storing number of backspaces needed to clear the command line
int hist_backspace;

// variable to check whether system call name should be displayed or not
extern int sysBool;

static void consputc(int);

static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];

  cli();
  cons.locking = 0;
  // use lapiccpunum so that we can call panic from mycpu()
  cprintf("lapicid %d: panic: ", lapicid());
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

static void
cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
  } else
    crt[pos++] = (c&0xff) | 0x0700;  // black on white

  if(pos < 0 || pos > 25*80)
    panic("pos under/overflow");

  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | 0x0700;
}

void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else
    uartputc(c);
  cgaputc(c);
}

#define INPUT_BUF 128
struct {
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} input;

#define C(x)  ((x)-'@')  // Control-x

void
consoleintr(int (*getc)(void))
{
  int c, doprocdump = 0;
  // var to store arrow code 
  int arrowcode = 0;

  acquire(&cons.lock);
  while((c = getc()) >= 0){
    // printint(c,10,0);  // for checking the code for arrow keys
    switch(c){
    case C('P'):  // Process listing.
      // procdump() locks cons.lock indirectly; invoke later
      doprocdump = 1;
      break;
    case C('U'):  // Kill line.
      while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input.e != input.w){
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    // if user presses control - x then accordingly changes sysBool variable to show names of system calls
    case C('S'):
        sysBool^=1;
        break;

    case ESC:
        // storing number of characters of command line and setting arrowcode pressed by user
        hist_backspace = (((input.e + INPUT_BUF) - input.r) % INPUT_BUF);
        c = getc();
        if(c == CSI){
          c = getc();
          if(c == UP_ARROW) 
            arrowcode = 1;
          else if(c == DOWN_ARROW) 
            arrowcode = 2;
        }
        break;

    default:
      if(c != 0 && input.e-input.r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;
        input.buf[input.e++ % INPUT_BUF] = c;
        consputc(c);
        if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
          history_ind = historyct+2;
          input.w = input.e;
          wakeup(&input.r);
        }
      }
      break;
    }
  }
  release(&cons.lock);
  if(doprocdump) {
    procdump();  // now call procdump() wo. cons.lock held
  }
  // calling history_arrow as per arrowcode
  // if arrowcode == 1 -> uparrow -> prev_history
  // if arrowcode == 2 -> downarrow -> next_history
  if(arrowcode == 1) 
    history_arrow(-1);
  else if(arrowcode == 2)  
    history_arrow(1);
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while(n > 0){
    while(input.r == input.w){
      if(myproc()->killed){
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  ioapicenable(IRQ_KBD, 0);
}

// storing command to histtemp buffer written on command line
void savehistory(char *cmd){
  // if in range of MAX_HISTORY_CT
  if(historyct < MAX_HISTORY_CT - 1) 
    historyct++;
  else{
    // if not in range delete the first stored command and make place to store new command
    for(int i = 0; i < MAX_HISTORY_CT - 1; i++){
      for(int j = 0; j <  128; j++){
        // first becomes second, second becomes third and so on...
        histtemp[i][j] = histtemp[i+1][j];
      }
    }
  }
  int j = 0;
  // storing command
  for(j = 0; cmd[j] != 0 && cmd[j] != '\n'; j++){
    histtemp[historyct][j] = cmd[j];
  }
  // terminating 
  histtemp[historyct][j] = 0;
}

// passing all saved commands through history variable
int gethistory(char* history){
  // cprintf("console.c %d:%p",__LINE__,hist); // debugging 
  int k = 0;
  for(int i = 0;i <= historyct; i++){
    for(int j = 0;j < 128; j++){
      history[k] = histtemp[i][j];
      k++;
    }
  }
  // cprintf("gethistory completed\n"); // debugging
  return historyct;
}

void history_arrow(int direction){
  // cprintf("HELlo\n");
  // setting history_ind to it's position as per arrow key
  if(direction == -1){ // up arrow key
      if(history_ind > 0){
        history_ind--;
      }
  }
  else{
    if(history_ind <= historyct) 
      history_ind++;
  }
  // if history_ind is in range of history stored
  if(history_ind <= historyct){
    // firstly erasing current command line command
    while((hist_backspace--) > 0){
      consputc(BACKSPACE);
    }
    input.r = 0;
    // copying history of hist_ind from histtemp and storing it in input.buf
    safestrcpy(input.buf, histtemp[history_ind], INPUT_BUF);
    input.e = strlen(histtemp[history_ind]);
    input.w = 0;
    // printing history on history_ind on console
    cprintf("%s", histtemp[history_ind]);
  }
}
