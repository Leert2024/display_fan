# include <string.h>
# include <stdio.h>
# include <stdarg.h>
# include "usart.h"
# include "serial.h"
# include "main.h"
# include "stm32f407xx.h"

/* USER CODE BEGIN SerialConfig */

/* 第1步: 如果使用DMA辅助传输，需要在此处extern DMA句柄(extern自usart.c) */
extern DMA_HandleTypeDef hdma_usart2_rx, hdma_usart2_tx;

/* 第2步: 将使用到的串口句柄(&huartx)填入此数组下标为x的位置，未使用的串口位置填入0 */
UART_HandleTypeDef* UART_HandleMap[ MAX_UART_PORT_NUM + 1 ] = {0, 0, &huart2, 0, 0, 0};

/* 第3步: 将使用到的串口DMA句柄(&hdma_usartx_rx, &hdma_usartx_tx)填入此数组下标为x的位置，未使用的位置填入0 */
DMA_HandleTypeDef* DMA_HandleMap[ MAX_UART_PORT_NUM + 1 ][2] = {
	{0, 0},
	{0, 0},
	{&hdma_usart2_rx, &hdma_usart2_tx},
	{0, 0},
	{0, 0},
	{0, 0}
};

// 第4步: 根据实际使用的串口，去HAL_UART_TxCpltCallback函数修改index的判断
// 第5步: 根据实际使用的串口，去HAL_UART_RxCpltCallback函数修改index的判断

/* USER CODE END SerialConfig */

/* 数组Serial_Buf存有所有的Serial_buf_t(串口缓冲队列)变量 */
Serial_Buf_t Serial_buf[MAX_UART_USED_NUM];

/* 串口对应的缓冲区在Serial_buf中的下标(仅在Serial_BufInit函数中可修改) */
uint8_t get_buf_index[7] = {MAX_UART_USED_NUM, MAX_UART_USED_NUM, MAX_UART_USED_NUM, MAX_UART_USED_NUM, MAX_UART_USED_NUM, MAX_UART_USED_NUM, MAX_UART_USED_NUM};

/* 串口透传映射表 */
uint8_t transparent_table[7][7] = {
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
};
uint8_t transparent_cnt = 0;

/* 串口镜像映射表 */
uint8_t mirror_table[7][7] = {
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
};
uint8_t mirror_cnt = 0;

volatile uint8_t is_eventloop = 0;

/**
 * @brief	判断特定序号的串口是否已被启用
 * @param	index 串口驱动的序号
 * @retval	已被启用返回1，否则返回0
 */
uint8_t Serial_IsUsed(uint8_t index){
	if(index > MAX_UART_PORT_NUM || index == 0)return 0;		// 是否在有效范围内
	if(get_buf_index[index] > MAX_UART_BUF_NUM - 1)return 0;	// 是否已分配缓冲区
	return 1;
}

/**
  * @brief	初始化特定串口的循环缓冲队列
  * @param	index 串口的编号
  * @param	doUseDMA 是否使用DMA
  * @retval	初始化成功返回1，否则返回0
  * @note	本函数被Serial_Init函数调用
  */
uint8_t Serial_BufInit( uint8_t index, uint8_t doUseDMA ){
	/* 为串口驱动分配缓冲区 */
	volatile static uint8_t buf_index = 0;
	if(buf_index >= MAX_UART_USED_NUM)return 0;
	get_buf_index[index] = buf_index;

	/* 初始化缓冲区 */
	memset(Serial_buf[buf_index].rx_buf, 0, UART_RX_BUF_SIZE);
	Serial_buf[buf_index].rx_buf_head = 0 ;	// 接收队列头指针置0
	Serial_buf[buf_index].rx_buf_tail = 0 ;	// 接收队列尾指针置0
	Serial_buf[buf_index].rx_buf_full = 0 ;	// 接收队列已满标志位置0
	Serial_buf[buf_index].rx_buf_full_irq = 0 ;	// 接收队列满中断标志位置0

	memset(Serial_buf[buf_index].tx_buf, 0, UART_TX_BUF_SIZE);
	Serial_buf[buf_index].tx_buf_head = 0 ;	// 发送队列头指针置0
	Serial_buf[buf_index].tx_buf_tail = 0 ;	// 发送队列尾指针置0
	Serial_buf[buf_index].tx_buf_full = 0 ;	// 发送队列已满标志位置0
	Serial_buf[buf_index].tx_busy = 0 ;		// 发送忙碌标志位置0

	Serial_buf[buf_index].doUseDMA = doUseDMA;

	buf_index++;
	return 1;
}

/**
  * @brief	开启特定序号的串口，并分配缓冲队列
  * @param	index 串口驱动的序号
  * @retval 开启成功返回1，失败返回0
  */
uint8_t Serial_Start(uint8_t index){
	/* 判断参数是否合法 */
	if ( index == 0 || index > MAX_UART_PORT_NUM )return 0;

	/* 判断是否在CubeMX中配置过USART */
	if(!(UART_HandleMap[index]->Instance->CR1 & USART_CR1_UE))return 0;

	/* 分配并初始化缓冲队列 */
	__IO uint8_t doUseDMA = (DMA_HandleMap[index][0]) ? 1 : 0;
	if(Serial_BufInit(index, doUseDMA) == 0)return 0;

	/* 开启串口空闲中断 */
	__HAL_UART_ENABLE_IT(UART_HandleMap[ index ], UART_IT_IDLE);
	if(doUseDMA)__HAL_UART_DISABLE_IT(UART_HandleMap[ index ], UART_IT_TXE);

	/* 进行DMA接收 */
	if(doUseDMA)HAL_UART_Receive_DMA(UART_HandleMap[index], Serial_buf[get_buf_index[index]].rx_buf[0], UART_RX_BUF_SIZE);
	else HAL_UART_Receive_IT(UART_HandleMap[index], Serial_buf[get_buf_index[index]].rx_buf[0], UART_RX_BUF_SIZE);

	return 1;
}

/**
 * @brief	停止串口中断模式接收
 * @param	huart UART句柄
 * @note	本函数在Uart_RxIDLE_Handler函数中被调用
 */
void UART_EndRxTransfer_IT(UART_HandleTypeDef *huart){
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

  /* At end of Rx process, restore huart->RxState to Ready */
  huart->RxState = HAL_UART_STATE_READY;
}

/**
 * @brief	停止串口DMA模式接收
 * @param	huart UART句柄
 * @note	本函数在Uart_RxIDLE_Handler函数中被调用
 */
void UART_DMAStopRx(UART_HandleTypeDef *huart){
	uint32_t dmarequest = 0x00U;
	dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR);
	if ((huart->RxState == HAL_UART_STATE_BUSY_RX) && dmarequest){
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);

    /* Abort the UART DMA Rx channel */
    if (huart->hdmarx != NULL)
    {
      HAL_DMA_Abort(huart->hdmarx);
    }
    UART_EndRxTransfer_IT(huart);
  	}
}

/**
  * @brief	关闭特定序号的串口异步收发功能
  * @param	index 串口编号
  * @retval 关闭成功返回1，失败返回0
  */
uint8_t Serial_Stop(uint8_t index){
	/* 检查参数是否合法，是否分配了缓冲区 */
	if ( index == 0 || index > MAX_UART_PORT_NUM ) return 0 ;
	uint8_t buf_index = get_buf_index[index];
	if (buf_index >= MAX_UART_USED_NUM) return 0;

	if (Serial_buf[buf_index].doUseDMA){
	   	UART_DMAStopRx( UART_HandleMap[ index ] );
	}else{
		UART_EndRxTransfer_IT( UART_HandleMap[ index ] );
	}

	__HAL_UART_DISABLE_IT(UART_HandleMap[ index ], UART_IT_IDLE);
	HAL_UART_DeInit(UART_HandleMap[ index ]) ;

	return 1 ;
}

/**
 * @brief   本函数对接收到的消息进行处理，由用户重写实现
 * @param   index 串口编号
 * @param   buf 缓冲区地址
 * @param   len 数据长度
 * @retval  返回值应为1。若返回0，消息会继续保留在接收缓冲队列中
 * @note	本函数将被CheckUartRxData函数调用
 */
__weak uint8_t RxDataCallback(uint8_t index, uint8_t* buf, uint32_t len){
	if(index == 1){
		// TODO
	}else if(index == 2){
		// TODO
	}else if(index == 3){
		// TODO
	}
	return 1;
}

/**
 * @brief   本函数对发送出的消息进行处理，由用户重写实现
 * @param   index 串口编号
 * @param   buf 缓冲区地址
 * @param   len 数据长度
 * @retval  返回值应为1。若返回0，消息会继续保留在发送缓冲队列中
 * @note	本函数将被UART_TxCpltCallback函数调用
 */
__weak uint8_t TxDataCallback(uint8_t index, uint8_t* buf, uint32_t len){
	if(index == 1){
		// TODO
	}else if(index == 2){
		// TODO
	}else if(index == 3){
		// TODO
	}
	return 1;
}

/**
 * @brief	接收队列满中断处理函数
 * @param	index UART编号(x)
 */
__weak void RxFullCallback(uint8_t index){}

/**
 * @brief	重写串口接收空闲中断处理函数
 * @param	index UART编号(x)
 * @note	本函数须要在stm32fxxx_it.c中被extern，并在USARTx_IRQHandler函数中被调用
 */
void UART_IDLECallback(uint8_t index){
	/* 清除标志位 */
	if( __HAL_UART_GET_FLAG( UART_HandleMap[ index ], UART_FLAG_IDLE) != RESET){
		__HAL_UART_CLEAR_IDLEFLAG( UART_HandleMap[ index ] );
		__HAL_UART_CLEAR_PEFLAG( UART_HandleMap[ index ] );
	}else return;	// 若未发生串口空闲中断，直接返回
	
	/* 检查串口是否分配了缓冲区 */
	uint8_t buf_index = get_buf_index[index];
	if(buf_index > MAX_UART_USED_NUM - 1)return;
	
	/* 停止接收 */
	uint8_t doUseDMA = Serial_buf[buf_index].doUseDMA;
	if(doUseDMA)UART_DMAStopRx(UART_HandleMap[ index ]);
	else UART_EndRxTransfer_IT(UART_HandleMap[ index ]);

	/* 计算接收到的长度 */
	uint16_t temp = UART_RX_BUF_SIZE;
	if(doUseDMA){
# ifdef STM32F1
	temp = DMA_HandleMap[ index ][0]->Instance->CNDTR;	// 获取DMA接收计数器的值
# endif
# ifdef STM32F4
	temp = DMA_HandleMap[ index ][0]->Instance->NDTR;	// 获取DMA接收计数器的值
# endif
	}
	else temp = UART_HandleMap[ index ]->RxXferCount ;
	uint16_t length = UART_RX_BUF_SIZE - temp;	// 计算接收到的数据长度

	/* 若接收到了消息 */
	if (length > 0){
		Serial_buf[ buf_index ].rx_buf_size[ Serial_buf[ buf_index ].rx_buf_tail ] = length;	// 记录接收到的数据长度
		if ( Serial_buf[ buf_index ].rx_buf_tail == ( MAX_UART_BUF_NUM - 1 ) )	// 如果尾指针指向缓冲队列的最后一个缓冲区
			Serial_buf[ buf_index ].rx_buf_tail = 0 ;	// 接收缓冲队列尾指针置0
		else Serial_buf[ buf_index ].rx_buf_tail ++ ;	// 如果接收缓冲队列没有满，尾指针加1
	}

	/* 如果接收缓冲队列已满，将接收缓冲队列已满标志位置1 */
	if ( length > 0 && Serial_buf[buf_index].rx_buf_tail == Serial_buf[buf_index].rx_buf_head ){
		Serial_buf[buf_index].rx_buf_full = 1 ;
		Serial_buf[buf_index].rx_buf_full_irq = 1 ;	// 接收缓冲队列满中断标志位置1
	}

	/* 重新启动串口接收 */
	if(Serial_buf[buf_index].doUseDMA)HAL_UART_Receive_DMA(UART_HandleMap[index], (uint8_t*)Serial_buf[buf_index].rx_buf[Serial_buf[buf_index].rx_buf_tail], UART_RX_BUF_SIZE);
	else HAL_UART_Receive_IT(UART_HandleMap[index], Serial_buf[buf_index].rx_buf[Serial_buf[buf_index].rx_buf_tail], UART_RX_BUF_SIZE);
}

/**
 * @brief   重写UART接收完成中断回调函数
 * @param   index 串口编号
 * @note	若该函数被调用，意味着单次收到了长度≥UART_RX_BUF_SIZE的消息
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart){
	uint8_t index = 0;

	/* 判断是哪个UART */
	if(huart->Instance == USART1) {
		index = 1 ;
	} else if(huart->Instance == USART2) {
		index = 2 ;
	} else if(huart->Instance == USART3) {
		index = 3 ;
	} else if(huart->Instance == UART4) {
		index = 4 ;
	} else if(huart->Instance == UART5) {
		index = 5 ;
	} else return ;

	/* 检查串口是否分配了缓冲区 */
	uint8_t buf_index = get_buf_index[index];
	if(buf_index > MAX_UART_BUF_NUM - 1)return;

	Serial_buf[buf_index].rx_buf_size[Serial_buf[buf_index].rx_buf_tail] = UART_RX_BUF_SIZE;	// 记录本次接收的数据长度
	if (Serial_buf[buf_index].rx_buf_tail == (MAX_UART_BUF_NUM - 1)){
		Serial_buf[buf_index].rx_buf_tail = 0;
	}else{
		Serial_buf[buf_index].rx_buf_tail ++ ;
	}

	/* 如果接收缓冲队列已满，将接收缓冲队列已满标志位置1 */
	if(Serial_buf[buf_index].rx_buf_tail == Serial_buf[buf_index].rx_buf_head){
		Serial_buf[buf_index].rx_buf_full = 1 ;
		Serial_buf[buf_index].rx_buf_full_irq = 1 ;	// 接收缓冲队列满中断标志位置1
	}

	/* 重新开启串口接收 */
	if(Serial_buf[buf_index].doUseDMA)HAL_UART_Receive_DMA(UART_HandleMap[index], Serial_buf[buf_index].rx_buf[Serial_buf[buf_index].rx_buf_tail], UART_RX_BUF_SIZE);
	else HAL_UART_Receive_IT(UART_HandleMap[index], Serial_buf[buf_index].rx_buf[Serial_buf[buf_index].rx_buf_tail], UART_RX_BUF_SIZE);
}

/**
 * @brief   重写UART发送完成中断回调函数
 * @param   index 串口编号
 * @note	该函数在发送完一条消息后会自动被DMA1_Channelx_IRQHandler调用
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart){
	uint8_t index = 0;

	/* 判断是哪个UART */
	if(huart->Instance == USART1) {
		index = 1 ;
	} else if(huart->Instance == USART2) {
		index = 2 ;
	} else if(huart->Instance == USART3) {
		index = 3 ;
	} else if(huart->Instance == UART4) {
		index = 4 ;
	} else if(huart->Instance == UART5) {
		index = 5 ;
	} else return ;

	/* 检查串口是否分配了缓冲区 */
	uint8_t buf_index = get_buf_index[index];
	if(buf_index > MAX_UART_BUF_NUM - 1)return;

	uint8_t* buf = Serial_buf[buf_index].tx_buf[Serial_buf[buf_index].tx_buf_head];
	uint32_t len = Serial_buf[buf_index].tx_buf_size[Serial_buf[buf_index].tx_buf_head];
	if(TxDataCallback(index, buf, len)){
		/* 原消息出队 */
		if(Serial_buf[buf_index].tx_buf_head == (MAX_UART_BUF_NUM - 1))Serial_buf[buf_index].tx_buf_head = 0 ;
		else Serial_buf[buf_index].tx_buf_head++;
	}

	Serial_buf[buf_index].tx_buf_full = 0;	// 发送缓冲队列已满标志位置0
	Serial_buf[buf_index].tx_busy = 0;		// 发送忙碌标志位置0
}

/**
 * @brief   通过串口发送数据(阻塞式)
 * @param   uart_index 串口的编号
 * @param   buf 要发送的数据的地址
 * @param   len 要发送的数据的长度(字节数)
 * @retval  发送成功返回1，失败返回0
 * @note    本函数不操作缓冲区，调用后立即进行发送，会产生阻塞直至发送完成
 */
uint8_t Serial_TxData( uint32_t index, uint8_t* buf, uint32_t len){
	/* 检查参数是否合法 */
	if(index > MAX_UART_PORT_NUM || index == 0)return 0;

	/* 调用HAL库函数进行阻塞式串口发送 */
	while(HAL_UART_Transmit(UART_HandleMap[index], buf, len, HAL_MAX_DELAY));

	/* 串口镜像 */
	for(uint8_t index2 = 0; index2 <= MAX_UART_PORT_NUM; index2++){
		if(mirror_table[index][index2]){
			while(HAL_UART_Transmit(UART_HandleMap[index2], buf, len, HAL_MAX_DELAY));
		}
	}

	return 1;
}

/**
 * @brief   通过串口发送格式化数据(阻塞式)
 * @param   uart_index 串口的编号
 * @param   format 格式化字符串
 * @param   ... 可变参数列表
 * @retval  发送成功返回1，失败返回0
 * @note    本函数不操作缓冲区，调用后立即进行发送，会产生阻塞直至发送完成
 */
uint8_t Serial_TxDataf( uint32_t index, char* format, ...){
	char buffer[UART_TX_BUF_SIZE];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args); // 将格式化字符串和可变参数列表转换为字符串
	va_end(args);
	return Serial_TxData(index, (uint8_t*)buffer, strlen(buffer));
}

static uint8_t tx_data_queue(uint32_t index, uint8_t* buf, uint32_t len){
	/* 检查参数是否合法 */
	if(len > UART_TX_BUF_SIZE)return 0;
	if(index > MAX_UART_PORT_NUM || index == 0)return 0;
	
	uint8_t buf_index = get_buf_index[index];
	if(buf_index > MAX_UART_BUF_NUM - 1)return 0;

	/* 检查发送缓冲队列是否已满 */
	if (Serial_buf[buf_index].tx_buf_full == 1 )return 0 ;

	/* 将数据从原地址拷贝到队列中的一个缓冲区里，并记录其长度 */
	memcpy(Serial_buf[buf_index].tx_buf[Serial_buf[buf_index].tx_buf_tail], buf, len);
	Serial_buf[ buf_index ].tx_buf_size[Serial_buf[buf_index].tx_buf_tail] = len;

	/* 更新队列尾指针 */
	if(Serial_buf[buf_index].tx_buf_tail == (MAX_UART_BUF_NUM - 1)){
		Serial_buf[buf_index].tx_buf_tail = 0;
	}else{
		Serial_buf[buf_index].tx_buf_tail++;
	}

	/* 如果发送缓冲队列已满，将队列已满标志位置1 */
	if ( Serial_buf[buf_index].tx_buf_tail == Serial_buf[buf_index].tx_buf_head){
		Serial_buf[buf_index].tx_buf_full = 1;
	}

	return 1;
}

/**
 * @brief   将数据添加到串口发送缓冲队列
 * @param   index 串口的编号
 * @param   buf 要发送的数据的地址
 * @param   len 要发送的数据的长度(字节数，不大于UART_TX_BUF_SIZE)
 * @retval  入队成功返回1，失败(参数不合法或队列已满)返回0
 * @note    本函数会将消息添加到UART发送缓冲队列中，但不会立即发送。在主循环中调用CheckUartTxData函数以检查缓冲队列并发送消息
 */
uint8_t Serial_TxDataQueue(uint32_t index, uint8_t* buf, uint32_t len){
  	if(tx_data_queue(index, buf, len) == 0)return 0;

	/* 串口镜像 */
	for(uint8_t index2 = 0; index2 <= MAX_UART_PORT_NUM; index2++){
		if(mirror_table[index][index2]){
			tx_data_queue(index2, buf, len);
		}
	}
	return 1;
}

/**
 * @brief   非阻塞格式化串口输出
 * @param   uart_index 串口的编号
 * @param   format 格式控制字符串
 * @param   ... 可变参数
 * @retval  入队成功返回1，失败(参数不合法或队列已满)返回0
 * @note    字符串长度不要超过UART_TX_BUF_SIZE
 */
uint8_t Serial_TxDataQueuef( uint32_t uart_index , const char *format, ...){
	char buffer[UART_TX_BUF_SIZE];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args); // 将格式化字符串和可变参数列表转换为字符串
	va_end(args);
	return Serial_TxDataQueue(uart_index, (uint8_t*)buffer, strlen(buffer));
}

/**
 * @brief	检查UART接收缓冲队列中是否有消息
 * @note	该函数在主循环中被调用，若检查到有消息，则消费队首的消息
 */
void CheckUartRxData(void){
	if(!is_eventloop)is_eventloop = 1;

	uint8_t buf_index;
 	for (int8_t index = 0; index <= MAX_UART_PORT_NUM; index++){
		buf_index = get_buf_index[index];
		if(buf_index > MAX_UART_USED_NUM - 1)continue;

		/* 检查接收队列满中断标志位 */
		if (Serial_buf[buf_index].rx_buf_full_irq == 1){
			Serial_buf[buf_index].rx_buf_full_irq = 0;	// 清除接收队列满中断标志位
			RxFullCallback(index);	// 调用接收队列满中断处理函数
		}
		
		if (Serial_buf[buf_index].rx_buf_head != Serial_buf[buf_index].rx_buf_tail || Serial_buf[buf_index].rx_buf_full == 1){
			int32_t len = Serial_buf[ buf_index ].rx_buf_size[ Serial_buf[ buf_index ].rx_buf_head ];
			uint8_t* rx_buf = Serial_buf[ buf_index ].rx_buf[ Serial_buf[ buf_index ].rx_buf_head ];

			/* 透传 */
			for(uint8_t index2 = 0; index2 <= MAX_UART_PORT_NUM; index2++){
				if(transparent_table[index][index2] == 1){
					Serial_TxDataQueue(index2, rx_buf, len);
				}
			}

			/* 镜像 */
			for(uint8_t index1 = 0; index1 <= MAX_UART_PORT_NUM; index1++){
				if(mirror_table[index1][index] == 1){
					RxDataCallback(index1, rx_buf, len);
				}
			}

			/* 如果接收回调函数对接收到的数据进行了消费，则将处理后的数据从缓冲队列中出队 */
			if(RxDataCallback(index, rx_buf, len)){
				/* 队首元素出队 */
				if(Serial_buf[buf_index].rx_buf_head == (MAX_UART_BUF_NUM - 1))Serial_buf[buf_index].rx_buf_head = 0;
				else Serial_buf[buf_index].rx_buf_head++;

				/* 接收缓冲队列已满标志位置0 */
				Serial_buf[buf_index].rx_buf_full = 0;
			}
		}
	}
}

/**
 * @brief  检查UART发送缓冲队列内是否有消息
 * @retval 无
 * @note   该函数在主循环中被调用，若检查到有消息，则发送队首的消息
 */
void CheckUartTxData( void ){
  	uint8_t buf_index;
 	for (int8_t index = 0; index <= MAX_UART_PORT_NUM; index++){
		/* 检查串口是否分配了缓冲区 */
		buf_index = get_buf_index[index];
		if(buf_index > MAX_UART_USED_NUM - 1)continue;

		/* 检查发送忙碌标志位 */
		if (Serial_buf[buf_index].tx_busy == 1)continue;

		/* 检查发送缓冲队列是否为空 */
		if (Serial_buf[buf_index].tx_buf_full == 1 || Serial_buf[buf_index].tx_buf_head != Serial_buf[buf_index].tx_buf_tail){
			uint8_t len = Serial_buf[buf_index].tx_buf_size[Serial_buf[buf_index].tx_buf_head];	// 获取要发送的数据长度
			uint8_t* tx_buf = Serial_buf[buf_index].tx_buf[Serial_buf[buf_index].tx_buf_head]; 	// 获取缓冲区地址
			Serial_buf[buf_index].tx_busy = 1;		// 发送忙碌标志位置1

			/* 发送数据 */
			if(Serial_buf[buf_index].doUseDMA)HAL_UART_Transmit_DMA(UART_HandleMap[index], tx_buf, len);
			else HAL_UART_Transmit_IT(UART_HandleMap[index], tx_buf, len);
		}
	}
}

/**
 * @brief	设置透传
 * @param	index1 源
 * @param	index2 转发目标
 * @param	status 透传状态(1:开启, 0:关闭)
 * @retval	设置成功返回1，否则返回0
 */
uint8_t Serial_SetTransparent(uint8_t index1, uint8_t index2, uint8_t status){
	if(!Serial_IsUsed(index1) || !Serial_IsUsed(index2))return 0;
	status = !!status;
	transparent_table[index1][index2] = status;
	if(status)transparent_cnt++; else transparent_cnt--;
	return 1;
}

/**
 * @brief	设置串口镜像
 * @param	index1 镜像源
 * @param	index2 转发目标
 * @param	status 转发状态(1:开启, 0:关闭)
 * @retval	设置成功返回1，否则返回0
 */
uint8_t Serial_SetMirror(uint8_t index1, uint8_t index2, uint8_t status){
	if(!Serial_IsUsed(index1) || !Serial_IsUsed(index2))return 0;
	status = !!status;
	mirror_table[index1][index2] = status;
	if(status)mirror_cnt++; else mirror_cnt--;
	return 1;
}
