//#include "types.h"
//#include "stat.h"
//#include "user.h"

//static void
//putc(int fd, char c)
//{
//  write(fd, &c, 1);
//}
//
//static void
//printint(int fd, int xx, int base, int sgn)
//{
//  static char digits[] = "0123456789ABCDEF";
//  char buf[16];
//  int i, neg;
//  uint x;
//
//  neg = 0;
//  if(sgn && xx < 0){
//    neg = 1;
//    x = -xx;
//  } else {
//    x = xx;
//  }
//
//  i = 0;
//  do{
//    buf[i++] = digits[x % base];
//  }while((x /= base) != 0);
//  if(neg)
//    buf[i++] = '-';
//
//  while(--i >= 0)
//    putc(fd, buf[i]);
//}
//
//// Print to the given fd. Only understands %d, %x, %p, %s.
//void
//printf(int fd, const char *fmt, ...)
//{
//  char *s;
//  int c, i, state;
//  uint *ap;
//
//  state = 0;
//  ap = (uint*)(void*)&fmt + 1;
//  for(i = 0; fmt[i]; i++){
//    c = fmt[i] & 0xff;
//    if(state == 0){
//      if(c == '%'){
//        state = '%';
//      } else {
//        putc(fd, c);
//      }
//    } else if(state == '%'){
//      if(c == 'd'){
//        printint(fd, *ap, 10, 1);
//        ap++;
//      } else if(c == 'x' || c == 'p'){
//        printint(fd, *ap, 16, 0);
//        ap++;
//      } else if(c == 's'){
//        s = (char*)*ap;
//        ap++;
//        if(s == 0)
//          s = "(null)";
//        while(*s != 0){
//          putc(fd, *s);
//          s++;
//        }
//      } else if(c == 'c'){
//        putc(fd, *ap);
//        ap++;
//      } else if(c == '%'){
//        putc(fd, c);
//      } else {
//        // Unknown % sequence.  Print it to draw attention.
//        putc(fd, '%');
//        putc(fd, c);
//      }
//      state = 0;
//    }
//  }
//}

#include "types.h"
#include "stat.h"
#include "user.h"
//
//void
//putc(int fd, const char c)
//{
//  write(fd, &c, sizeof(c));
//}
//
//static void
//printint(int fd, int xx, int base, int sign)
//{
//  static char charset[] = "0123456789abcdef";
//  char buf[16];
//  int i;
//  uint x;
//
//  if (sign && (sign = xx < 0)) {
//    x = -xx;
//  } else {
//    x = xx;
//  }
//
//  i = 0;
//  do {
//    buf[i++] = charset[x % base];
//  } while ((x /= base) != 0);
//
//  if (sign)
//    buf[i++] = '-';
//
//  while (--i >= 0)
//    putc(fd, buf[i]);
//}
//
//// Print to the given fd. Only understands %d, %x, %p, %s.
//void
//printf(int fd, const char *fmt, ...)
//{
//  char *s;
//  int c, i, state = 0;
//  uint *ap = (uint*)(void*)&fmt + 1;
//
//  for (i = 0; fmt[i]; i++) {
//    c = fmt[i] & 0xff;
//
//    if (state == 0) {
//      if (c == '%') {
//        state = '%';
//      } else {
//        putc(fd, c);
//      }
//    } else if (state == '%') {
//      if (c == 'd') {
//        printint(fd, *ap, 10, 1);
//        ap++;
//      } else if (c == 'x' || c == 'p') {
//        printint(fd, *ap, 16, 0);
//        ap++;
//      } else if (c == 's') {
//        s = (char*) *ap;
//        ap++;
//
//        if (s == 0)
//          s = "(null)";
//
//        while (*s != 0) {
//          putc(fd, *s);
//          s++;
//        }
//      } else if (c == 'c') {
//        putc(fd, *ap);
//        ap++;
//      } else if (c == '%') {
//        putc(fd, c);
//      } else {
//        // Unknown % sequence.  Print it to draw attention.
//        putc(fd, '%');
//        putc(fd, c);
//      }
//
//      state = 0;
//    }
//  }
//}

// Print to the given fd. Only understands %d, %x, %p, %s, %c (%c is already present in the code)
// Buffered-printf, saves strings to memory and then writes to the fd when bufsize is reached or strings ends
#define printf_BUFSIZE 128

static void
bputc(int fd, char c, char *pbuf, int *bi)
{
  pbuf[bi[0]++] = c;

  if (bi[0] >= printf_BUFSIZE) {
    write(fd, pbuf, printf_BUFSIZE);
    memset(pbuf, 0, printf_BUFSIZE);
    bi[0] = 0;
  }
}

static void
printint(int fd, int xx, int base, int sign, char *pbuf, int *bi)
{
  static char charset[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if (sign && (sign = xx < 0)) {
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do {
    buf[i++] = charset[x % base];
  } while((x /= base) != 0);

  if (sign)
    buf[i++] = '-';

  while (--i >= 0)
    bputc(fd, buf[i], pbuf, bi);
}

void
printf(int fd, const char *fmt, ...)
{
  char pbuf[printf_BUFSIZE];
  int bi = 0;

  char *s;
  int c, i, state = 0;
  uint *ap = (uint*)(void*)&fmt + 1;

  for (i = 0; fmt[i]; i++) {
    c = fmt[i] & 0xff;

    if (state == 0) {
      if (c == '%') {
        state = '%';
      } else {
        bputc(fd, c, pbuf, &bi);
      }
    } else if (state == '%') {
      if (c == 'd') {
        printint(fd, *ap, 10, 1, pbuf, &bi);
        ap++;
      } else if (c == 'x' || c == 'p') {
        printint(fd, *ap, 16, 0, pbuf, &bi);
        ap++;
      } else if (c == 's') {
        s = (char*) *ap;
        ap++;

        if (s == 0)
          s = "(null)";

        while (*s != 0) {
          bputc(fd, *s, pbuf, &bi);
          s++;
        }
      } else if (c == 'c') {
        bputc(fd, *ap, pbuf, &bi);
        ap++;
      } else if (c == '%') {
        bputc(fd, c, pbuf, &bi);
      } else {
        // Unknown % sequence.  Print it to draw attention.
        bputc(fd, '%', pbuf, &bi);
        bputc(fd, c, pbuf, &bi);
      }

      state = 0;
    }
  }

  if (bi > 0) {
    write(fd, pbuf, bi);
  }
}

#undef bprintf_BUFSIZE
