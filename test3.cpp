#include "malloc_3.cpp"
int main(){
void * a = smalloc(12);
void* b = smalloc(1234);
sfree(a);
sfree(b);
a = smalloc (1234);
sfree(a);
	return 0;
}
