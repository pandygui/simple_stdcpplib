#define USE_STDPERIPH_DRIVER

#include "stm32f10x.h"
#include "stm32_p103.h"

#include "bst.h"
#include "mem.h"
#include "myiostream.h"
#include "mystring.h"
#include "myvec.h"

#if 0
#include "k_stdio.h"
#include "s_eval.h"
#include "gdeque.h"
#include "cstring.h"
#endif

using namespace DS;

uint8_t get_byte()
{
  while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);

  return (USART_ReceiveData(USART2) & 0x7F);
}

#if 0
void myfree(void *ptr)
{
}

void *mymalloc(int size)
{
  static char mem[256];
  return mem;
}
#endif

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

void test_bst()
{
  int i=0;

  GNode<int, int> *root = 0;
  for (i=0 ; i < 1000 ; ++i)
  {
    cout << "insert:" << endl;
    root = insert(root, 8, 7);
  }
  //root = insert(root, 4, 7);
  //root = insert(root, 14, 7);
  print_tree(root);
}

void test_vec()
{
  DS::vector<int> vec;

  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  vec.push_back(4);
  vec.push_back(5);
  vec.push_back(6);
  for (int i=0 ; i < vec.length() ; ++i)
  {
    cout << vec[i] << endl;
  }
  cout << vec.max_size() << endl;
}

extern "C"
{

int main(void)
{

  //extern int __start_global_ctor__;
  //extern int __end_global_ctor__;

  int val=98;
  double d_val=3.56;

  init_rs232();
  USART_Cmd(USART2, ENABLE);


  //char *p1 = (char *)mymalloc(4);
#if 0
  char *p1 = new char;
  print_memarea();
  delete p1;
  print_memarea();
#endif

#if 0
  {
    string str("i am mystring");
  }

  printf("test bst: %d, d_val: %f\r\n", val, d_val);

  cout << "test bst: " << val << endl;
#endif
  int i=100;
  print_memarea();
  TRY
  {
    test_vec();
    //test_bst();
  }
  CATCH(NOFREE_MEM)
  {
    cout << "no mem, i: " << i << endl;
  }
  ETRY

  // print_memarea();
  while(1);


  cout << endl;

  {
    GNode<double, int> *root = 0;

    cout << "sizeof(double): " << sizeof(double) << endl;

    root = insert(root, 8.1, 7);
    root = insert(root, 4.2, 7);
    root = insert(root, 14.3, 7);
    print_tree(root);
    print_memarea();
  }

  while(1);

  cout << endl;


  {
    GNode<DS::string, int> *root = 0;
    root = insert(root, "def", 7);
    root = insert(root, "xyz", 5);
    root = insert(root, "abc", 8);
    print_tree(root);
  }

  while(1);

  return 0;

}

}
