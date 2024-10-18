#ifndef CONFIG_H
#define CONFIG_H

// User agent for YR.no - If you're using this for your own project, change the user agent to avoid getting blocked by YR.no
#define USER_AGENT_PERSONAL "epaper/1.0 [YOUR_GITHUB_USERNAME]"

// If false, the graph will be replaced with a message saying "Det blir opphald dei neste 90 minutta"
#define SHOW_GRAPH_ON_NO_PRECIPITATION true

#define BATTERY_MAX_VOLTAGE 4.2  // Charging voltage
#define BATTERY_MIN_VOLTAGE 2.75 // Discharge voltage

#define SHOW_BATTERY_INDICATOR true

#endif // CONFIG_H