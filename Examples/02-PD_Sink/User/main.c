
#include "debug.h"
#include "PD_Process.h"

/* Global define */

/* Global Variable */

/*********************************************************************
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   Initializes GPIOA.0
 *
 * @return  none
 */
void GPIO_Toggle_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint32_t _board_tick = 0;
void board_tick_init(void){

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM1, ENABLE );

    TIM_TimeBaseInitStructure.TIM_Period = 1000-1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 48-1;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00;
    TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure);
    TIM_ClearITPendingBit( TIM1, TIM_IT_Update );
    NVIC_SetPriority(TIM1_UP_IRQn, 15);
    NVIC_EnableIRQ(TIM1_UP_IRQn);
    TIM_ITConfig( TIM1, TIM_IT_Update, ENABLE );
    TIM_Cmd( TIM1, ENABLE );
}

uint32_t board_get_tick(void)
{
    uint32_t tick;
    TIM1->DMAINTENR &= (uint16_t)~TIM_IT_Update;
    tick = _board_tick;
    TIM1->DMAINTENR |= TIM_IT_Update;
    return tick;
}

void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void)
{
    _board_tick++;
    TIM_ClearITPendingBit( TIM1, TIM_IT_Update );
}


void board_usbpd_init(void) {
    PD_Init();
}

void board_usbpd_loop(uint32_t tick_now) {
    static uint32_t tick_pd = 0;

    Tmr_Ms_Dlt = tick_now - tick_pd;
    tick_pd = tick_now;
    PD_Ctl.Det_Timer += Tmr_Ms_Dlt;
    if (PD_Ctl.Det_Timer > 4) {
        PD_Ctl.Det_Timer = 0;
        PD_Det_Proc();
    }
    PD_Main_Proc();
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    u8 i = 0;
    uint32_t tick_now = 0;
    uint32_t tick_blink = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    board_tick_init();
    GPIO_Toggle_INIT();

    board_usbpd_init();
    while(1)
    {
        tick_now = board_get_tick();

        board_usbpd_loop(tick_now);

        if(tick_now >= tick_blink)
        {
            tick_blink = tick_now + 500;
            GPIO_WriteBit(GPIOB, GPIO_Pin_12, (i == 0) ? (i = Bit_SET) : (i = Bit_RESET));
        }

    }
}
