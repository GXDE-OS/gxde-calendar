# gxde-calendar

Calendar for GXDE Desktop Environment.

## Features

- **Lunar Calendar Support**: Automatically displays traditional Chinese lunar calendar information including lunar months, days, and Gan-Zhi (Stem-Branch) dates
- **Festival Recognition**: Highlights Chinese traditional festivals and solar terms (Jieqi) with distinct visual indicators
- **Holiday Management**: Integrates with online holiday API to show workday adjustments (work on weekends, holidays on weekdays) and workday status
- **Multi-Theme Support**: Adapts to system light/dark themes with customizable color schemes for calendar cells
- **Weekday Configuration**: Supports configurable first day of the week (Monday or Sunday)
- **Daily Inspiration**: Fetches and displays daily motivational quotes or meaningful sentences
- **DBus Integration**: Provides DBus interface for raising the main window and inter-process communication
- **Single Instance Mode**: Ensures only one calendar instance runs at a time, bringing existing window to front when relaunched

## Innovation Highlights

- **Hybrid Calendar Engine**: Seamlessly combines Gregorian dates with accurate lunar calendar calculations through DBus service integration, providing both modern and traditional date references in a unified view
- **Intelligent Workday Detection**: Dynamically determines work/holiday status by fetching real-time government holiday schedules and automatically overlaying them onto the standard calendar grid
- **Elegant Festival Highlighting**: Distinct visual styling for lunar festivals, solar terms, and national holidays, making important dates immediately recognizable at a glance
- **Adaptive Cell Rendering**: Smart color schemes that differentiate between regular weekdays, weekends, festivals, and current day, with separate text and background color management for lunar information
- **Offline-First Holiday Data**: Initial holiday dataset embedded locally, with seamless online updates when network connectivity is available — ensuring consistent operation in disconnected environments
