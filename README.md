<h1 align="center">FreeRTOS Tickless Idle + Interrupt‑Driven Sensor System</h1>



A FreeRTOS‑based firmware system for the STM32F411RE Nucleo board demonstrating:



- Task‑based architecture  

\- Interrupt‑driven wakeups  

\- Direct task notifications  

\- Semaphores for asynchronous events  

\- Message queues for UART serialisation  

\- FreeRTOS \*\*tickless idle + WFI sleep mode\*\*  

\- HAL‑based SHT31 temperature/humidity sensor driver  



This project is designed to showcase \*\*real RTOS fundamentals\*\*, \*\*deterministic task behaviour\*\*, and \*\*low‑power operation\*\*, written cleanly for recruiters and engineering teams.



The system uses `configSYSTICK\_CLOCK\_HZ = CPU / 8`, increasing the SysTick overflow window from \*\*1.048 seconds (16 MHz)\*\* to \*\*8.32 seconds (2 MHz)\*\*.  

A periodic TIM3 interrupt every \*\*4 seconds\*\* resets the SysTick counter before overflow, ensuring stable tickless‑idle timing.



Race conditions are eliminated by \*\*avoiding shared resources\*\*, and priority inversion is prevented by using a \*\*message queue\*\* for UART serialisation.  

The SHT31 driver intentionally uses \*\*integer‑based measurement\*\* to avoid floating‑point overhead inside RTOS tasks.



\---



\## 🚀 Project Overview



This firmware implements:



\- \*\*Timer‑driven periodic measurements\*\* (TIM3 → task notification)  

\- \*\*SHT31 sensor read task\*\* (HAL I2C)  

\- \*\*EXTI13 button interrupt task\*\* (binary semaphore)  

\- \*\*UART serialisation task\*\* (queue‑based)  

\- \*\*FreeRTOS tickless idle\*\* with `PreSleepProcessing()` + `\_\_WFI()`  

\- \*\*Deterministic task wakeup timing\*\*  

\- \*\*Clear task priority hierarchy\*\*  



The design is intentionally simple, deterministic, and hardware‑centric — ideal for demonstrating RTOS fundamentals.



\---



\## 🧩 Task Architecture



\### \*\*1. Timer Wake Task (Priority 5)\*\*

\- Wakes on TIM3 interrupt  

\- Pulses PA6  

\- Notifies the sensor task  



\### \*\*2. Sensor Task (Priority 4)\*\*

\- Reads SHT31 via HAL I2C  

\- Formats temperature/humidity  

\- Sends message pointer to UART queue  

\- Pulses PA7  



\### \*\*3. EXTI Task (Priority 3)\*\*

\- Wakes on EXTI13 interrupt (button press)  

\- Sends “Button pressed!” message  

\- Pulses PA8  



\### \*\*4. UART Task (Priority 2)\*\*

\- Serialises all messages over USART2  

\- Pulses PA9  



\---



\## 🔗 Inter‑Task Communication



| Mechanism | Used For | Reason |

|----------|----------|--------|

| \*\*Direct Task Notification\*\* | TIM3 → timerWakeTask → sensorTask | Fastest RTOS signalling |

| \*\*Binary Semaphore\*\* | EXTI ISR → extiTask | Ideal for asynchronous external events |

| \*\*Message Queue\*\* | sensorTask + extiTask → uartTask | Deterministic UART serialisation |



\---



\## 💤 Tickless Idle + Sleep Mode



Tickless idle is enabled:



\- `configUSE\_TICKLESS\_IDLE = 1`  

\- `HAL\_SuspendTick()` before sleep  

\- `\_\_WFI()` executed  

\- `HAL\_ResumeTick()` after wake  



\### ⭐ Important  

The system enters \*\*Sleep mode\*\*, providing meaningful low‑power behaviour:



\- SysTick disabled  

\- CPU sleeps until next interrupt  

\- No periodic tick wakeups  

\- Deterministic wake timing  



\---



\## 📡 Interrupts



\### TIM3 Interrupt

\- Sends direct task notification  

\- Wakes timerWakeTask  

\- Drives periodic sensor reads  



\### EXTI13 Interrupt

\- Gives semaphore  

\- Wakes extiTask  

\- Handles button events  

&#x20; 

\---



\## 📂 Repository Structure



```text

tickless-idle-freeRTOS/

├── Core/                    # Application source \& include files

├── Drivers/                 # CMSIS and HAL driver files

├── Docs/                    # Logic analyser \& Serial monitor screenshots, architecture, power-notes

├── Middlewares/             # FreeRTOS files  

├── Project files/           # CubeMX .IOC file

├── LICENSE                  # MIT License

└── README.md                # Project configuration description

```



\---



\## 🏗 Build Instructions



\### Build (CubeIDE)

\- Open project  

\- Build → Debug  

\- Flash to Nucleo board  



\---



\## 🎯 Why This Project Matters



This project demonstrates:



\- Real FreeRTOS experience  

\- Task‑based architecture  

\- Interrupt‑driven design  

\- Direct task notifications  

\- Semaphores and queues  

\- Tickless idle low‑power behaviour  

\- HAL I2C sensor integration  

\- Clean, readable firmware structure  

\- Professional documentation  



It reflects the engineering practices expected in embedded/firmware roles across the UK.



\---



\## 📜 License



MIT License — free to use, modify, and build upon.

