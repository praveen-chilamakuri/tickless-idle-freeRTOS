# Power Behaviour Notes

This project uses **FreeRTOS tickless idle** combined with the ARM `WFI` instruction to reduce power consumption during idle periods.

---

## Sleep Mode

The firmware enters **normal Sleep mode**:

> `__WFI()` → **Sleep mode**  
> Clocks remain active  
> SysTick is suspended  
> Wakeup occurs on interrupts (TIM3, EXTI13)

---

## Tickless Idle Behaviour

Tickless idle is enabled:

- `configUSE_TICKLESS_IDLE = 1`
- `HAL_SuspendTick()` before sleep
- `HAL_ResumeTick()` after wake

This reduces power by:
- Removing periodic SysTick interrupts  
- Allowing longer uninterrupted sleep windows  
- Waking only on real events (timer or EXTI)

---

## Sleep Entry Sequence

1. FreeRTOS detects long idle period  
2. Calls `PreSleepProcessing()`  
3. HAL tick suspended  
4. PA10 set HIGH (debug marker)  
5. CPU executes `__WFI()`  
6. MCU enters Sleep mode

---

## Sleep Exit Sequence

1. Interrupt occurs (TIM3 or EXTI13)  
2. CPU wakes  
3. FreeRTOS calls `PostSleepProcessing()`  
4. HAL tick resumed  
5. PA10 set LOW  
6. Scheduler continues normally

---

## Power Advantages

- No SysTick overhead during idle  
- Sleep windows up to several milliseconds  
- Deterministic wakeup behaviour  
- No PLL or clock reconfiguration required  
- Safe for HAL‑based projects

---

## Future Improvements

To achieve deeper power savings:
- Use `STOP mode`  
- Disable unused peripherals  
- Use LPTIM instead of TIM3  
- Replace HAL I2C with LL driver for lower overhead  

These enhancements would bring the project closer to ultra‑low‑power design.
