/*
 * sys_arch.c
 *
 *  Created on: Jul 2, 2014
 *      Author: yonch
 */

#include <arpa/inet.h>
#include "cc.h"

u16_t lwip_htons(u16_t x) {
	return htons(x);
}
u16_t lwip_ntohs(u16_t x) {
	return ntohs(x);
}
u32_t lwip_htonl(u32_t x) {
	return htonl(x);
}
u32_t lwip_ntohl(u32_t x) {
	return ntohl(x);
}
