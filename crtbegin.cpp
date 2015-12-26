#include "stm32.h"
#include "crtbegin.h"
#include "type.h"
#include "eh.h"
#include "mem.h"
#include "myvec.h"
#include "myiostream.h"

#ifdef P103
#include "stm32f10x.h"
#include "stm32_p103.h"
#endif

#ifdef STM32F407
#include "stm32f407_io.h"
#endif


using namespace DS;

#if 0

void init_command()
{
   int i=0;
   for(i=0;i<MAX_COMMAND_LEN;i++)
      command_buff[i]='\0';
}
#endif






#define STACK_SIZE 0xfff0
extern unsigned long _etext;
extern unsigned long _data;
extern unsigned long _edata;
extern unsigned long _bss;
extern unsigned long _ebss;

void *operator new(unsigned int s)
{
  void *ptr = mymalloc(s);

  if (ptr == 0)
    THROW(NOFREE_MEM);

  return ptr;
}

void *operator new[](unsigned int s)
{
  //cout << "s: " << s << endl;
  // printf("s: %d\r\n", s);

  void *ptr = mymalloc(s);

  if (ptr == 0)
    THROW(NOFREE_MEM);

  return ptr;
}

void operator delete(void *p)
{
  myfree(p);
}

void operator delete[](void *p)
{
  myfree(p);
}


void *__dso_handle;

static int obj_count=0;

DS::vector<DObjs> dobjs_vec;

int ex_code;

void g_dtor()
{
#if 0
  for (int i=obj_count-1 ; i >= 0 ; --i)
    dobjs[i].dtor_(dobjs[i].arg_);
#endif

  for (int i=0 ; i < dobjs_vec.length() ; ++i)
  {
    dobjs_vec[i].dtor_(dobjs_vec[i].arg_);
  }
}

extern "C"
{
  int __cxa_atexit(void (*destructor) (void *), void *arg, void *__dso_handle)
  {
    DObjs dobj;

    dobj.dtor_ = destructor;
    dobj.arg_ = arg;
    dobj.dso_handle_ = __dso_handle;

    dobjs_vec.push_back(dobj);

    DS::printf("fill ctor data: obj_count: %d, arg:%d\r\n", obj_count, arg);
    ++obj_count;
  }

  // ref: http://git.opencores.org/?a=viewblob&p=uClibc&h=ebb233b4fce33b04953f1d9158a0479487bb50db&f=libc/sysdeps/linux/arm/aeabi_atexit.c
  /* Register a function to be called by exit or when a shared library
     is unloaded.  This routine is like __cxa_atexit, but uses the
     calling sequence required by the ARM EABI.  */
  int __aeabi_atexit (void *arg, void (*func) (void *), void *d);
  int __aeabi_atexit (void *arg, void (*func) (void *), void *d)
  {
    return __cxa_atexit (func, arg, d);
  }
}

typedef void (*Fp)();

void enter_main()
{
  // init usart for showing error message
 
  // stm32f407
#ifdef STM32F407
  init_usart(9600);
#endif

  // p103
#ifdef P103
  init_rs232();
  USART_Cmd(USART2, ENABLE);
#endif

  // ur_puts(USART2, "Init complete! Hello World!\r\n");

  TRY
  {

    extern unsigned int __start_global_ctor__;
    extern unsigned int __end_global_ctor__;
    unsigned int *start = &__start_global_ctor__;
    unsigned int *end = &__end_global_ctor__;

    // run global object ctor
    for (unsigned int *i = start; i != end; ++i)
    {
      Fp fp = (Fp)(*i);
      (*fp)();
    }

    mymain();
  }
  CATCH(NOFREE_MEM)
  {
    cout << "no mem" << endl;
  }
  ETRY
  // run global object dtor
}

void ResetISR(void)
{
  unsigned long *pulSrc, *pulDest;

  pulSrc = &_etext;
  for (pulDest = &_data; pulDest < &_edata;)
    *pulDest++ = *pulSrc++;
  for (pulDest = &_bss; pulDest < &_ebss;)
    *pulDest++ = 0;

  enter_main();
}

void pendsv_isr(void)
{
  int i=5;

  ++i;
}

void svc_isr(void)
{
  // pendsv set:
  //ref: @ Cortex™-M3 Technical Reference Manual (file name: DDI0337E_cortex_m3_r1p1_trm.pdf) p8-19
  *( unsigned long *) 0xE000ED04 |= 0x10000000;
}

void systick_isr(void)
{
}

void int_isr(void)
{
}

#ifdef EXT_INT

void wwdg_isr(void)
{
  int b=33;
}
void pvd_isr(void)
{
}
void tamp_stamp_isr(void)
{
}

void rtc_wkup_isr(void)
{
}
void flash_isr(void)
{
}
void rcc_isr(void)
{
}

void exti0_isr(void);
#endif

typedef void (*pfnISR)(void);

static unsigned long pulStack[10] __attribute__((section(".stackares")));
 __attribute__((section(".ccm")))
static u8 stack[STACK_SIZE];


__attribute__((section(".isr_vector")))
pfnISR VectorTable[]=
{
  (pfnISR)((unsigned long)stack + STACK_SIZE),
  ResetISR, // 1
  int_isr,
  int_isr,
  int_isr,
  int_isr,
  int_isr,
  int_isr,
  int_isr,
  int_isr,
  int_isr,
  svc_isr,    // 11
  int_isr,
  int_isr,
  pendsv_isr, // 14
  systick_isr, // 15

#ifdef EXT_INT
  // External Interrupts
  wwdg_isr,                   // Window WatchDog
  pvd_isr,                   // PVD through EXTI Line detection                      
  tamp_stamp_isr,            // Tamper and TimeStamps through the EXTI line
  rtc_wkup_isr,              // RTC Wakeup through the EXTI line                     
  flash_isr,                 // FLASH                                           
  rcc_isr,                   // RCC                                             
  exti0_isr                  // EXTI Line0 
#endif
};