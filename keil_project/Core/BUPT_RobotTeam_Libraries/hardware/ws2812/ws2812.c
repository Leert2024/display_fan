# include "ws2812.h"
# include "tim.h"

# define MY_WS2812_MAX_NUM 39	// WS2812灯带LED数量
# define MY_WS2812_RST_NUM 300	// WS2812复位信号长度
# define MY_WS2812_BUF_LEN (MY_WS2812_RST_NUM + MY_WS2812_MAX_NUM * 3 * 8)	// WS2812 DMA缓冲区总长度
# define WS2812_SET (67)	// 逻辑1对应的PWM值(0.80us高电平)
# define WS2812_RSET (34)	// 逻辑0对应的PWM值(0.40us高电平)
# define WS2812_htim htim4	// WS2812定时器4
# define WS2812_TIM_CHANNEL TIM_CHANNEL_3	// WS2812定时器通道3

/* WS2812 DMA缓冲区 */
uint16_t MY_WS2812_GRB_BUF[MY_WS2812_BUF_LEN] = {0};

/**
 * @brief	直接写WS2812 DMA缓冲区（不含复位信号）
 * @param	content 要写入的内容（uint16_t*）
 * @note	确保写入的内容有效长度不小于MY_WS2812_BUF_LEN
 */
void WS2812_SetBuf(uint16_t* content){
	memcpy(MY_WS2812_GRB_BUF, content, MY_WS2812_MAX_NUM * 3 * 8 * 2);
}

/**
 * @brief 设置指定LED的颜色
 * @param num LED编号
 * @param rv 红色分量值(0-255)
 * @param gv 绿色分量值(0-255)
 * @param bv 蓝色分量值(0-255)
 */
void WS2812_SetSingle(uint16_t num, uint8_t rv, uint8_t gv, uint8_t bv){
	for (uint8_t i = 0; i < 8; i++){
		MY_WS2812_GRB_BUF[MY_WS2812_RST_NUM + num * 24 + i]      = (gv << i) & (0x80) ? WS2812_SET : WS2812_RSET;
		MY_WS2812_GRB_BUF[MY_WS2812_RST_NUM + num * 24 + i + 8]  = (rv << i) & (0x80) ? WS2812_SET : WS2812_RSET;
		MY_WS2812_GRB_BUF[MY_WS2812_RST_NUM + num * 24 + i + 16] = (bv << i) & (0x80) ? WS2812_SET : WS2812_RSET;
	}
}

/**
 * @brief 关闭所有LED
 */
void WS2812_AllOff(void){ 
	for(uint16_t i = 0; i < MY_WS2812_MAX_NUM; i++){
	   WS2812_SetSingle(i, 0, 0, 0);
	}
}

/**
 * @brief 打开所有LED(设置为白色)
 */
void WS2812_AllOn(void){
	for(uint16_t i = 0; i < MY_WS2812_MAX_NUM; i++){
	   WS2812_SetSingle(i, 255, 255, 255);
	}
}

/**
 * @brief  启动WS2812的DMA传输
 * @retval 无
 */
void WS2812_Refresh(void){
    HAL_TIM_PWM_Start_DMA(&WS2812_htim, WS2812_TIM_CHANNEL, (uint32_t *)MY_WS2812_GRB_BUF, sizeof(MY_WS2812_GRB_BUF) / sizeof(uint16_t));
}

/**
 * @brief 初始化WS2812
 * @note 启动PWM DMA传输
 */
void WS2812_Init(void){
	WS2812_AllOff();
    WS2812_Refresh();
}
