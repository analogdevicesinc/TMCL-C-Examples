/*******************************************************************************
* Copyright © 2018 TRINAMIC Motion Control GmbH & Co. KG
* (now owned by Analog Devices Inc.),
*
* Copyright © 2024 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

//  Created on: 15.02.2018	Author: ED

#ifndef SYSTICK_H
#define SYSTICK_H

	#include "Definitions.h"

	void systick_init(void);
	unsigned int systick_getTimer(void);

#endif
