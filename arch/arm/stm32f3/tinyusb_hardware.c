/*
 * tinyusb_hardware.c: STM32F3 USB hardware init for TinyUSB.
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stm32f3xx.h"
#include "tusb.h"

void tinyusb_hardware_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    (void) RCC->APB1ENR;

    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER11_Msk | GPIO_MODER_MODER12_Msk))
                 | (GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1);
    GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR11 | GPIO_OSPEEDER_OSPEEDR12);
    GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPDR11 | GPIO_PUPDR_PUPDR12);
    GPIOA->AFR[1]  &= ~((0xFU << 12) | (0xFU << 16));
    GPIOA->AFR[1]  |= (14U << 12) | (14U << 16);

    NVIC_SetPriority(USB_HP_CAN_TX_IRQn, 1);
    NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, 1);
    NVIC_EnableIRQ(USB_HP_CAN_TX_IRQn);
    NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);
    NVIC_EnableIRQ(USBWakeUp_IRQn);
}

void USB_HP_CAN_TX_IRQHandler(void)     { tud_int_handler(0); }
void USB_LP_CAN_RX0_IRQHandler(void)    { tud_int_handler(0); }
void USBWakeUp_IRQHandler(void)         { tud_int_handler(0); }
