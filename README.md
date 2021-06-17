<html xmlns="http://www.w3.org/1999/xhtml" lang xml:lang>
<head>
  <meta charset="utf-8" />
  <meta name="generator" content="pandoc" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=yes" />
  <meta name="author" content />
  <!--[if lt IE 9]>
    <script src="//cdnjs.cloudflare.com/ajax/libs/html5shiv/3.7.3/html5shiv-printshiv.min.js"></script>
  <![endif]-->
</head>
<body>
<header id="title-block-header">
<h1 class="title">CESLA Firmware - Sleep Variant</h1>
<p class="author"></p>
</header>
<nav id="TOC" role="doc-toc">
<ul>
<li><a href="#overview">Overview</a></li>
<li><a href="#configuration">Configuration</a></li>
<li><a href="#verification">Verification</a></li>
<li><a href="#design">Design</a></li>
<li><a href="#communication">Communication</a></li>
</ul>
</nav>

<section id="overview" class="level1">
<h1>Overview</h1>
<p>This program implements the CESLA firmware with deep sleep modes on the earable device.</p>
<p>This firmware is meant to be used with the RSL10-SENSE-DB-GEVK evaluation board to acquire sound signals from the analog microphones and communicate with the CESLA Android mobile application.</p>


![Available operation modes and transition events between them.](./.readme-res/app_flowchart.png?raw=true "Available operation modes and transition events between them.") 
<figcaption>Available operation modes and transition events between them.</figcaption>

<p>This application can enter one of the following states:</p>
<ul>
<li><p><strong>Advertising mode</strong> <br> Default state after power up. All stream providers are disabled in this mode. Board periodically sends advertising packets (1s interval by default) to allow other devices to connect to it.</p>
<p>If no device connects in defined timeout period (default 60s) the device will enter sleep mode. Switch to sleep mode is indicated by red LED blinking once.</p></li>
<li><p><strong>Sleep mode</strong> <br> In this mode all BLE advertising activity is stopped and all sensors are disabled.</p>
<p>The RSL10 wakes up periodically (default 1.5s) to check if button PB1 is pressed. If button is pressed it will enter Advertising mode signaled by short blink of green LED.</p></li>
<li><p><strong>Connected Mode</strong> <br> This mode can be entered from Advertising mode when a device connects to the board. BLE connection will be maintained based on parameters set by master device.</p>
<p>In this mode stream providers can be turned on if master requests sensor related data:</p>
<ul>
<li><p><em>LCA</em> - Left channel audio provider will be enabled after a LCA stream request is received.</p></li>
<li><p><em>RCA</em> - Right channel audio provider will be enabled after a LCA stream request is received.</p></li>
<li><p><em>DMIC</em> - Digital mirophone audio provider will be enabled after a LCA stream request is received.</p></li>
</ul>
<p>When peer device disconnects the board will enter back into advertising mode.</p></li>
</ul>

</section>

<section id="configuration" class="level1">
<h1>Configuration</h1>
<p>Certain parameters of the application can be configured using an RTE configuration header. This file is located in the <em>include</em> folder of the application and is called <em>RTE_app_config.h</em>. After opening this file using the <em>CMSIS Configuration Wizard</em> editor it will show all available configuration options.</p>

![Selecting CMSIS Configuration Wizard to open the configuration file.](./.readme-res/rte_app_config.jpg?raw=true "Selecting CMSIS Configuration Wizard to open the configuration file.")

<figcaption>Selecting CMSIS Configuration Wizard to open the configuration file.</figcaption>

![All configurable parameters of this application as shown in CMSIS Configuration Wizard](./.readme-res/configurable_parameters_sleep.png?raw=true "All configurable parameters of this application as shown in CMSIS Configuration Wizard")

<figcaption>All configurable parameters of this application as shown in CMSIS Configuration Wizard</figcaption>


</section>

<section id="verification" class="level1">
<h1>Verification</h1>
<p>After the program is flashed and running on RSL10-SENSE-GEVK board it will start connectable advertising over BLE. The BLE name is 'CESLA_BLE_Terminal'. The CESLA Android application or test suite can be used to receive the audio samples and show the data. </p>

<p>The program exposes the following audio stream providers in the application: 
<ul>
<li>Left channel audio stream</li>
<li>Right channel audio stream</li>
<li>On-board digital microphone audio stream</li>
</ul>
</p>

Features extraction stream is not yet implemented in firmware. 

![cesla_base_firmware_setup](./.readme-res/cesla_base_firmware_setup.jpg?raw=true "cesla_base_firmware_setup")
<figcaption>cesla_base_firmware_setup</figcaption>

</section>

</section>

<section id="design" class="level1">
<h1>Design</h1>

![High Level Software Design](./.readme-res/Software-Design.png?raw=true "High Level Software Design")

<figcaption>High Level Software Design</figcaption>
</section>

<section id="communication" class="level1">
<h1>Communication</h1>
This program implements a custom service BLE GATT service profile. The GATT database and client-server interactions are described in the following images. 

![GATT Data Structure](./.readme-res/GATT-Data-Structure.png?raw=true "GATT Data Structure")
<figcaption>GATT Data Structure</figcaption>

![GATT Client Operations](./.readme-res/GATT-Client-Operations.png?raw=true "GATT Client Operations")
<figcaption>GATT Client Operations</figcaption>
The frame number in the above figure represents a 2-byte timestamp. The timestamp is only included when the debug bit in the SCP op-code is set. Otherwise, 10 2-byte audio samples are transmitted. 
</section>


</body>
</html>
