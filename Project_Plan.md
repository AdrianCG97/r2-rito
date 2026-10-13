# R2-Rito ROBOT

### Goal
Build an autonomous omnidirectional mobile robot from the ground up, progressing from off-the-shelf development boards and low-level motor/sensor drivers to a custom hardware PCB, embedded RTOS, custom Linux SBC image, and ROS 2-based SLAM navigation.

---

## High-Level Plan

### 1) Low-Level Firmware & Control (Yahboom MCU Board)
  - [ ] Get printf working
    - [ ] Test basic
    - [ ] Use DMA and test
  - [ ] Heartbeat
  - [ ] Implement closed-loop DC motor velocity control (PID) and encoder feedback
  - [ ] Derive and implement 4-wheel omnidirectional kinematic model
    - *Reference:* [Omnidirectional Wheeled Mobile Robots (Modern Robotics)](https://modernrobotics.northwestern.edu/nu-gm-book-resource/13-2-omnidirectional-wheeled-mobile-robots-part-1-of-2/)
  - [ ] Select and configure RTOS:
    - Start with **FreeRTOS** for fast prototyping and motor loop determinism
    - Evaluate migrating to **Zephyr RTOS** for native DeviceTree and modular drivers
  - [ ] Establish host-to-MCU communication interface (UART/USB CDC or CAN using micro-ROS or lightweight packet protocol)
  - Board info: https://www.yahboom.net/study/ROS-Driver-Board

### 2) High-Level Compute & ROS 2 Environment
  - [ ] Platform Selection:
    - **Phase A (Prototyping):** Offboard Laptop over Wi-Fi for fast debugging and compute headroom
    - **Phase B (Onboard Autonomy):** Raspberry Pi 4/5 (or Jetson Nano) mounted directly to the chassis
  - [ ] Configure ROS 2 workspace (Humble/Jazzy)
  - [ ] Implement base controller node (translating `cmd_vel` to wheel velocities and publishing `/odom` + TF)

### 3) Perception, Sensor Fusion & SLAM
  - [ ] Hardware integration:
    - 2D LiDAR (360° planar scan)
    - Depth camera or stereo vision
    - 6-DOF / 9-DOF IMU on MCU
  - [ ] Sensor Fusion:
    - Implement Extended Kalman Filter (`robot_localization`) fusing wheel odometry and IMU data
  - [ ] SLAM & Navigation:
    - 2D LiDAR SLAM using `slam_toolbox`
    - Visual-inertial odometry / RGB-D SLAM exploration (e.g., RTAB-Map)
    - Autonomous waypoint navigation via Nav2

### 4) Custom Hardware & PCB Design
  - [ ] Schematic capture and component selection (STM32 MCU, motor drivers, power regulation)
  - [ ] Dedicated debug and programming interfaces:
    - SWD header with SWO trace pin broken out
    - USB-C interface for data/power
  - [ ] Power distribution network:
    - Battery management, motor power rail, and isolated 5V/3.3V logic rails
  - [ ] Board layout, routing, DRC, and manufacturing order

### 5) Custom Embedded Linux OS
  - [ ] Set up custom BSP using **Yocto Project** or **Buildroot** for the onboard SBC
  - [ ] Strip unnecessary desktop overhead for minimal boot time and deterministic operation
  - [ ] Configure systemd services to auto-launch micro-ROS agent and core ROS 2 stack on boot
  - [ ] Implement headless telemetry and over-the-air (OTA) update mechanism

---

## Detailed Task Execution Plan

Tasks are ordered strictly by dependency. Complete each phase and verify its milestone before moving to the next.


### Phase 1: Low-Level Hardware Bringup & Actuation (Yahboom Board)

  - [ ] **1.1 Peripheral & Timer Configuration**
    - [ ] Configure hardware timers for multi-channel PWM generation (10–20 kHz) to drive motor H-bridges without audible coil whine.
    - [ ] Configure hardware quadrature encoder timer inputs in 4x counting mode (`TIM_ENCODERMODE_TI12`) for all drive motors.
    - [ ] Implement encoder overflow/underflow interrupt handling to maintain global 32-bit/64-bit tick counts.

  - [ ] **1.2 Low-Level Control Loops**
    - [ ] Implement discrete-time PID controllers with anti-windup clamping for each wheel motor.
    - [ ] Run motor velocity control loops inside a deterministic hardware timer interrupt (target: 100 Hz to 1 kHz).
    - [ ] Tune PID gains on the bench (proportional response, steady-state error elimination, step-response verification).

  - [ ] **1.3 Kinematics Implementation**
    - [ ] Implement inverse kinematics: map body velocity targets ($v_x, v_y, \omega$) to individual wheel target velocities.
    - [ ] Implement forward kinematics: compute body frame displacements from individual wheel encoder tick deltas.
    - [ ] Verify kinematics on blocks (ensure wheels spin in the correct directions for forward, strafing, and pure rotational commands).


### Phase 2: RTOS Migration (FreeRTOS)

  - [ ] **2.1 Kernel Initialization & Architecture**
    - [ ] Integrate FreeRTOS with static memory allocation (`configSUPPORT_STATIC_ALLOCATION = 1`) to eliminate heap fragmentation risks.
    - [ ] Configure SysTick, interrupt priority grouping, and verify RTOS context-switching determinism.

  - [ ] **2.2 Task Decomposition & Thread Safety**
    - [ ] Task 1 (High Priority, 100 Hz): Actuator loop — read encoders, run PID, update PWM output.
    - [ ] Task 2 (Medium Priority, 50 Hz): Telemetry loop — forward kinematics computation and odometry packaging.
    - [ ] Task 3 (Medium Priority, Event-Driven): Communication parsing — decode incoming motion packets and push targets to thread-safe ring buffers.
    - [ ] Task 4 (Low Priority, 10 Hz): System health monitoring, battery voltage ADC sampling, and heartbeat LED.
    - [ ] Implement inter-task communication using FreeRTOS queues, mutexes with priority inheritance, and task notifications.


### Phase 3: Host-to-MCU Communication Layer

  - [ ] **3.1 Physical Interface & Protocol Setup**
    - [ ] Configure serial communications (UART via USB CDC or CAN-FD transceiver depending on Yahboom breakout).
    - [ ] Implement packet framing, serialization, and error detection (e.g., COBS encoding with CRC32, or raw CAN-FD frame IDs).
    - [ ] Define communication schema:
      - Downlink (`Host -> MCU`): Target velocities ($v_x, v_y, \omega$), enable/disable flags, emergency stop.
      - Uplink (`MCU -> Host`): Wheel positions/velocities, calculated odometry, battery status, fault codes.

  - [ ] **3.2 Host Communication Node**
    - [ ] Write a C++ communication driver node on the host computer to handle bidirectional byte streaming.
    - [ ] Implement communication watchdog: MCU enters safe state (zero velocity) if no valid packet arrives within a timeout window (e.g., 200 ms).


### Phase 4: High-Level ROS 2 Base Integration

  - [ ] **4.1 Workspace & Base Driver**
    - [ ] Set up ROS 2 workspace (Humble or Jazzy).
    - [ ] Implement the base interface node subscribing to `/cmd_vel` (`geometry_msgs/msg/Twist`) and publishing `/odom` (`nav_msgs/msg/Odometry`).
    - [ ] Broadcast base coordinate transforms using `tf2_ros` (`odom` -> `base_footprint` -> `base_link`).

  - [ ] **4.2 Teleoperation & Open-Loop Validation**
    - [ ] Integrate `teleop_twist_keyboard` or a gamepad node.
    - [ ] Validate physical ground movement: verify linear translation distances and rotational angles match physical measurements to calibrate wheel radius and base geometry.


### Phase 5: Sensors, Odometry Fusion & SLAM

  - [ ] **5.1 Sensor Integration**
    - [ ] Mount and configure 2D planar LiDAR; bring up vendor ROS 2 driver node (`sensor_msgs/msg/LaserScan`).
    - [ ] Mount IMU (or read onboard MCU IMU over telemetry); publish calibrated `sensor_msgs/msg/Imu` data.
    - [ ] Define rigid transform tree (`URDF` / robot state publisher) establishing spatial offsets between `base_link`, LiDAR, and IMU.

  - [ ] **5.2 Sensor Fusion (EKF)**
    - [ ] Configure `robot_localization` (`ekf_node`) to fuse wheel odometry and IMU angular velocity/linear acceleration.
    - [ ] Verify that wheel slippage during aggressive acceleration or strafing does not catastrophically drift the fused `/odometry/filtered` frame.

- [ ] **5.3 Mapping & Navigation**
  - [ ] Configure and run `slam_toolbox` in synchronous online mapping mode to generate 2D occupancy grids.
  - [ ] Tune Nav2 stack (costmaps, AMCL localization, controller/planner servers for omnidirectional holonomic drive).
  - [ ] Execute autonomous point-to-point waypoint navigation with obstacle avoidance in a dynamic environment.


### Phase 6: Custom Hardware / PCB Design

  - [ ] **6.1 System Architecture & Schematics**
    - [ ] Select primary microcontroller (STM32G4 or STM32H7 family) and peripheral allocations.
    - [ ] Select gate drivers / integrated H-bridge ICs rated for motor stall currents.
    - [ ] Design power architecture: wide-input buck regulators for 5V/3.3V logic rails, reverse polarity protection, and flyback suppression.
    - [ ] Integrate external peripherals: CAN-FD transceiver, USB-C ESD protection, IMU on high-speed SPI bus.
    - [ ] Add standard 10-pin/20-pin Cortex SWD programming header with SWO trace support and accessible test points.

  - [ ] **6.2 Layout, Verification & Assembly**
    - [ ] Route PCB with dedicated power planes, star ground / split analog-digital grounding, and short high-current motor loops.
    - [ ] Run design rule checks (DRC), generate Gerber/BOM files, and fabricate PCB.
    - [ ] Assemble board, verify power rail voltages without MCU populated, and validate flashing via ST-Link/J-Link over SWD.
    - [ ] Port the Phase 1–3 firmware stack to the custom board hardware.


### Phase 7: Custom Embedded Linux OS (SBC)

- [ ] **7.1 Build System Setup**
  - [ ] Set up a custom Yocto Project (Poky) or Buildroot layer target for the onboard SBC (e.g., Raspberry Pi Compute Module or alternative).
  - [ ] Strip out desktop GUI, audio, and unnecessary background services to create a headless, stripped-down distribution.
  - [ ] Compile real-time kernel (`PREEMPT_RT`) and configure target hardware interfaces (UART, SPI, CAN overlays).

- [ ] **7.2 Deployment & Productionization**
  - [ ] Bake the target ROS 2 core packages directly into the root filesystem image via `meta-ros` (or containerized via headless Docker).
  - [ ] Set up systemd services to auto-launch micro-ROS/hardware nodes and sensor drivers on boot.
  - [ ] Implement an A/B partition layout or read-only rootfs with an overlay partition for robust power-loss resilience.


