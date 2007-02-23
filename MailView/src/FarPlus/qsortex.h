#ifndef ___QSORTEX_H___
#define ___QSORTEX_H___

#ifdef __cplusplus
extern "C" {
#endif

typedef int (__cdecl comp_t)(const void *, const void *, void *);
void __cdecl qsortex( void * base, unsigned num, unsigned width,
	comp_t *comp, void * );


#ifdef __cplusplus
}
#endif

#endif //!defined(___QSORTEX_H___)
