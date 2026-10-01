# Architecture

This project implements a FreeRTOS‑based firmware system running on the STM32F411RE Nucleo board.  
It demonstrates task‑based design, interrupt‑driven wakeups, inter‑task communication, and FreeRTOS tickless idle for low‑power operation.

The architecture is intentionally simple, deterministic, and hardware‑centric — ideal for showcasing RTOS fundamentals to embedded engineering teams and recruiters.

---

## System Components

### 1. Timer Wake Task (`timerWakeTask`)
- Wakes when TIM3 interrupt sends a **direct task notification**  
- Pulses PA6 for timing/debug  
- Notifies the **sensor task** to perform a measurement  
- Priority: **5** (highest)

This task acts as the system’s periodic trigger.

---

### 2. Sensor Task (`sensorTask`)
- Waits for notification from the timer task  
- Pulses PA7
- Reads SHT31 sensor using HAL I2C  
- Formats temperature, humidity into a message
- Sends message pointer to UART queue  
- Priority: **4**

This task performs the main application workload.

---

### 3. EXTI Task (`extiTask`)
- Waits on a **binary semaphore** given from EXTI13 ISR  
- Pulses PA8  
- Sends “Button pressed!” message to UART queue  
- Priority: **3**

This task handles asynchronous external events.

---

### 4. UART Task (`uartTask`)
- Receives message pointers from queue  
- Serialises all output over USART2  
- Pulses PA9  
- Priority: **2**

This task ensures clean, ordered UART output.

---

## Inter‑Task Communication

### ✔ Direct Task Notifications  
Used between:
- TIM3 ISR → `timerWakeTask`
- `timerWakeTask` → `sensorTask`

Fast, lightweight, zero‑copy signalling.

---

### ✔ Binary Semaphore  
Used between:
- EXTI ISR → `extiTask`

Ideal for asynchronous external events.

---

### ✔ Message Queue  
Used between:
- `sensorTask` → `uartTask`
- `extiTask` → `uartTask`

Queue stores **pointers to static message buffers**, ensuring:
- No dynamic allocation  
- No copying large strings  
- Deterministic behaviour  

---

## Tickless Idle + Sleep Mode

FreeRTOS tickless idle is enabled:

- `configUSE_TICKLESS_IDLE = 1`
- `configSYSTICK_CLOCK_HZ = CPU / 8`
- `HAL_SuspendTick()` before sleep
- `__WFI()` executed in `PreSleepProcessing`
- `HAL_ResumeTick()` after wake

This results in:

> **The MCU enters Sleep mode**  
> Clocks remain active, SysTick is suspended, and wakeup occurs on interrupts.

---

## Execution Flow

### 1. TIM3 interrupt  
→ wakes `timerWakeTask`  
→ triggers `sensorTask`

### 2. Sensor task  
→ reads SHT31  
→ sends formatted message to UART queue

### 3. EXTI13 interrupt  
→ wakes `extiTask`  
→ sends “Button pressed!” message

### 4. UART task  
→ serialises all messages over USART2

### 5. Idle periods  
→ FreeRTOS tickless idle  
→ HAL tick suspended  
→ CPU enters Sleep via WFI  
→ wakes on next interrupt

---


