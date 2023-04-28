#include "lwip_hooks.h"
#include "opt.h"
#include "lwip/sys.h"
#include "string.h"
#ifdef LWIP_HOOK_FILENAME
#include LWIP_HOOK_FILENAME
#endif

#ifdef LWIP_HOOK_TCP_ISN
//only random value without using the ip and port of local and remote machine
void lwip_init_tcp_isn()
{
    /*nop, the lwip_hook_tcp_isn get by random, no using the secret keys*/
}

u32_t lwip_hook_tcp_isn(ip_addr_t *local_ip, u16_t local_port, ip_addr_t *remote_ip, u16_t remote_port)
{
    srand(lega_rtos_get_time());
    return (u32_t)rand();
}
#endif
