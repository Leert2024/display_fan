# WS2812

## 使用
### CubeMX配置
1. 配置引脚为TIMx_CHy
2. 左侧栏目选Timers -> TIMx，Channel y选PWM Generation CHy
3. Parameter Settings中，自动重装载寄存器设为89，自动重装载预装载使能
4. DMA Settings增加一项，DMA Request为TIMx_CHy，方向为Memory to Peripheral
5. NVIC勾选TIMx global interrupt

### 源码配置
需设置`MY_WS2812_MAX_NUM`为实际使用到的灯带上的LED数量

## 性能
有N颗灯珠的WS2812的理论最高刷新频率为：
> f = 1s / 1.25us * N + 50us

