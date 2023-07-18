# hbeat-bachelor-project

This project aimed to develop plugins for OBS Studio streaming program that provide visual representations of the user's heartbeat. The plugins are designed to display the heartbeat information in three different ways:

Number Display: This plugin shows the actual value of the heartbeat as a simple numerical readout.

Heart GIF Animation: This plugin utilizes a GIF animation of a heart that dynamically changes its size according to the value of the heartbeat.

Real-time Graph: This plugin presents a graphical representation of the heartbeat that updates in real time.

The heartbeat data is collected using the labstreaminglayer library, which provides the necessary functionality for capturing and processing the heartbeat information.

Usage:
In OBS Studio, create or open the scene where you want to display the heartbeat plugin.

Add a new source to the scene and select the corresponding heartbeat plugin from the available options.

Configure the plugin settings, such as the position, size, and appearance, according to your preferences.

Ensure that the labstreaminglayer library is running and properly configured to collect the heartbeat data.

Start streaming or recording in OBS Studio to visualize the heartbeat information in real time.
