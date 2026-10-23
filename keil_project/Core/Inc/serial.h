# ifndef __SERIAL_H__
# define __SERIAL_H__

# include "main.h"

# define UART_RX_BUF_SIZE    128  // 单个接收缓冲区的大小
# define UART_TX_BUF_SIZE    128  // 单个发送缓冲区的大小
# define MAX_UART_BUF_NUM    4   // 缓冲队列含有的Buffer数量(即最多存储的消息条数)

# define MAX_UART_PORT_NUM   6
# define MAX_UART_USED_NUM   5

# pragma pack(4)

/* 循环缓冲队列 */
typedef struct{
	/* 发送队列 */
	uint8_t	tx_buf[ MAX_UART_BUF_NUM ][ UART_TX_BUF_SIZE ];	// 发送队列
	uint32_t tx_buf_size[ MAX_UART_BUF_NUM ];    // 发送队列中各个缓冲区存有的字节数
	volatile uint32_t tx_buf_head;	// 发送队列头指针
	volatile uint32_t tx_buf_tail;	// 发送队列尾指针
	volatile uint32_t tx_buf_full;	// 发送队列满标志位
	volatile uint32_t tx_busy;		// 发送忙碌标志位

    /* 接收队列 */
	uint8_t rx_buf[ MAX_UART_BUF_NUM ][ UART_RX_BUF_SIZE ];	// 接收队列
	uint32_t rx_buf_size[ MAX_UART_BUF_NUM ];    // 接收队列中各个缓冲区存有的字节数
	volatile uint32_t rx_buf_head;	// 接收队列头指针
	volatile uint32_t rx_buf_tail;	// 接收队列尾指针
	volatile uint32_t rx_buf_full;	// 接收队列满标志位
	volatile uint8_t rx_buf_full_irq;	// 接收队列满中断标志位

	/* 是否使用DMA */
	volatile uint8_t doUseDMA;
}Serial_Buf_t;

/* 串口配置 */

uint8_t Serial_Start(uint8_t index);
uint8_t Serial_Stop(uint8_t index);
uint8_t Serial_IsUsed(uint8_t index);
uint8_t Serial_SetTransparent(uint8_t index1, uint8_t index2, uint8_t status);
uint8_t Serial_SetMirror(uint8_t index1, uint8_t index2, uint8_t status);

/* 串口发送 */

uint8_t Serial_TxData(uint32_t index, uint8_t* buf, uint32_t len);
uint8_t Serial_TxDataf( uint32_t index, char* format, ...);
uint8_t Serial_TxDataQueue(uint32_t index, uint8_t* buf, uint32_t len);
uint8_t Serial_TxDataQueuef( uint32_t uart_index , const char *format, ...);

/* 串口任务(应置于事件循环中) */

void CheckUartTxData( void );
void CheckUartRxData( void );

/* 待用户实现的消息解析函数 */

__weak uint8_t TxDataCallback(uint8_t index, uint8_t* buf, uint32_t len);
__weak uint8_t RxDataCallback(uint8_t index, uint8_t* buf, uint32_t len);
__weak void RxFullCallback(uint8_t index);

# endif
