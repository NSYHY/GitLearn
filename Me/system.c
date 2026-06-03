#include "system.h"

int fputc(int ch,FILE *str)
{
	HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,10);
	return ch;
}

uint16_t tir_i = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim -> Instance == TIM1)
	{
		key_tick ++;
		tir_i ++;
	}
}

uint32_t value;
uint32_t fre;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim -> Instance == TIM17)
	{
		value = TIM17 -> CCR1;
		TIM17 -> CNT = 0;
		fre = SystemCoreClock/((TIM17 -> PSC) * value);
	}
}

uint32_t dma_buff_1[30];//ADC1 R38
uint32_t dma_buff_2[30];//ADC2 R37
float adc1;
float adc2;
//void adc_show(void)
//{
//	for(uint8_t i=0; i<= 30; i++)
//	{
//		adc1 += (float)dma_buff_1[i];
//		adc2 += (float)dma_buff_2[i];
//	}
//	adc1 = (adc1/30)*(3.3f/4096);
//	adc2 = (adc2/30)*(3.3f/4096);
//}
void adc_show(void)
{
    static float window1[30] = {0};   // 保存最近30个原始ADC值
    static float window2[30] = {0};
    static float sum1 = 0;            // 窗口内累加和
    static float sum2 = 0;
    static uint8_t idx = 0;           // 当前要覆盖的窗口位置
    static uint8_t filled = 0;        // 窗口是否已满（用于初始化）

    float new_sample1 = (float)dma_buff_1[0];
    float new_sample2 = (float)dma_buff_2[0];

    if (filled) {
        sum1 -= window1[idx];
        sum2 -= window2[idx];
    }

    window1[idx] = new_sample1;
    window2[idx] = new_sample2;
    sum1 += new_sample1;
    sum2 += new_sample2;

    // 移动索引
    idx++;
    if (idx >= 30) {
        idx = 0;
        filled = 1;  
    }

    // 计算平均值
    uint8_t count = filled ? 30 : idx;
    float avg1 = sum1 / count;
    float avg2 = sum2 / count;

    // 转换为电压
    const float factor = 3.3f / 4096.0f;
    adc1 = avg1 * factor;
    adc2 = avg2 * factor;
}



/*写*/
void eeprom_write(uint8_t addr,uint8_t dat)
{
	I2CStart();//开启
	I2CSendByte(0xa0);//发送字节执行写
	I2CSendAck();//等待应答
	I2CSendByte(addr);//写入地址
	I2CSendAck();//等待应答
	I2CSendByte(dat);//写入数据
	I2CSendAck();//等待应答
	I2CStop();//停止
	HAL_Delay(15);
}

/*读*/
uint8_t eeprom_read(uint8_t addr)
{
	I2CStart();//开启
	I2CSendByte(0xa0);//发送字节执行写
	I2CSendAck();//等待应答
	I2CSendByte(addr);//地址数据
	I2CSendAck();//等待应答
	I2CStop();//停止
	
	I2CStart();//开启
	I2CSendByte(0xa1);//发送字节执行读
	I2CSendAck();//等待应答
	uint8_t dat = I2CReceiveByte();
	I2CSendNotAck();
	I2CStop();//停止
	return dat;
}

void system_init(void)
{
	LCD_Init();
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	I2CInit();
	printf("5");
	eeprom_write(2,100);
	feng = eeprom_read(0);
	
	led_disp(0x00);
}
