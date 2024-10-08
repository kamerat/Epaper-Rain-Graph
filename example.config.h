#ifndef CONFIG_H
#define CONFIG_H

// User agent for YR.no - If you're using this for your own project, change the user agent to avoid getting blocked by YR.no
#define USER_AGENT_PERSONAL "epaper/1.0 [YOUR_GITHUB_USERNAME]"

// YR location id for Bergen (Find it by the url when you search for the location on yr.no [https://www.yr.no/nn/vêrvarsel/dagleg-tabell/1-92416/Noreg/Vestland/Bergen/Bergen])
#define YR_LOCATION "1-92416"

// Debug mode
#define DEBUG false

// If false, the graph will be replaced with a message saying "Det blir opphald dei neste 90 minutta"
#define SHOW_GRAPH_ON_NO_PRECIPITATION true

#endif // CONFIG_H