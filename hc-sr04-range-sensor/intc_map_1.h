/*
 * Copyright (C) 2021 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the
 *        distribution.
 *
 *      * Neither the name of Texas Instruments Incorporated nor the names of
 *        its contributors may be used to endorse or promote products derived
 *        from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _INTC_MAP_1_H_
#define _INTC_MAP_1_H_

/*
 * ======== PRU INTC Map ========
 *
 * Define the INTC mapping for interrupts going to the ICSS / ICSSG:
 * 	ICSS Host interrupts 0, 1
 * 	ICSSG Host interrupts 0, 1, 10-19
 *
 * Note that INTC interrupts going to the ARM Linux host should not be defined
 * in this file (ICSS/ICSSG Host interrupts 2-9).
 *
 * The INTC configuration for interrupts going to the ARM host should be defined
 * in the device tree node of the client driver, "interrupts" property.
 * See Documentation/devicetree/bindings/interrupt-controller/ti,pruss-intc.yaml
 * entry #interrupt-cells for more.
 *
 * For example, on ICSSG:
 *
 * &client_driver0 {
 * 	interrupt-parent = <&icssg0_intc>;
 * 	interrupts = <21 2 2>, <22 3 3>;
 * 	interrupt-names = "interrupt_name1", "interrupt_name2";
 * };
 *
 */

#include <stddef.h>
#include <rsc_types.h>

/*
 * .pru_irq_map is used by the RemoteProc driver during initialization. However,
 * the map is NOT used by the PRU firmware. That means DATA_SECTION and RETAIN
 * are required to prevent the PRU compiler from optimizing out .pru_irq_map.
 */

#if !defined(__GNUC__)
  #pragma DATA_SECTION(my_irq_rsc, ".pru_irq_map")
  #pragma RETAIN(my_irq_rsc)
  #define __pru_irq_map		/* */
#else
  #define __pru_irq_map __attribute__((section(".pru_irq_map"),unavailable("pru_irq_map is for usage by the host only")))
#endif

struct pru_irq_rsc my_irq_rsc __pru_irq_map = {
	0,			/* type = 0 */
	1,			/* number of system events being mapped */
	{
		{19, 1, 1},	/* {sysevt, channel, host interrupt} */
	},
};

#endif /* _INTC_MAP_1_H_ */
