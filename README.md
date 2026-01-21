

Intelligent Logistics Engine (SwiftEx) 📦 🚚
Course: Data Structures & Algorithms (CSC-200L)

Institution: University of Engineering and Technology (UET), Lahore

A comprehensive C++ console application designed to simulate a modern courier logistics system. This project integrates robust data structures to handle intelligent parcel sorting, optimal route calculation, real-time tracking, and rider management. It features a CLI interface coupled with a live web-based visualization dashboard

🚀 Features
Core Operations
Role-Based Access: Secure Login and Registration system for Admins and Customers.

Smart Dispatching: Creates parcel requests with specific weights and priorities (Standard, Express, Urgent).

Intelligent Routing: Calculates all available paths between cities using DFS and recommends the shortest route based on distance.

Priority Sorting: Automatically processes high-priority and urgent parcels first using a Max-Heap.

Live Tracking: Provides a detailed timeline of events (Pickup -> Warehouse -> In Transit -> Delivered) for every parcel.

Advanced Capabilities
Web Visualization: Launches a local Python HTTP server to display a live, graphical map of cities, roads, and moving parcels in your browser (monitor.html).

Dynamic Traffic: Simulates random road blockages, forcing the system to re-route parcels in real-time.

Undo System: Allows admins to revert recent actions (like creating or dispatching a parcel) using a Stack.

Investigation Protocol: Handles missing parcels with an automated investigation simulation and insurance claim generation.

Graph Management: Admins can add new cities, build roads, or block existing routes dynamically.

🧠 Data Structures Used
This project is built using custom implementations of the following data structures:

Map Network: Implemented using a Graph (Adjacency Lists) to manage city connections, distances, and road statuses.
Parcel Sorting: Uses a Max-Heap (Priority Queue) to sort parcels by priorityScore to ensure urgent deliveries go first.
Database: Utilizes a Hash Table with Quadratic Probing to provide $O(1)$ access for Parcel IDs and User credentials
Transit System: Uses a Linked List to manage the lifecycle of parcels currently "In Transit" or "Loading".
Rider Fleet: Implemented with a Queue to manage rider availability in a round-robin fashion.
Undo Log: Uses a Stack to store recent operations, allowing for "Undo" functionality.
Storage: Custom ArrayLists are used for efficient dynamic arrays for edges, paths, and object lists.

🛠️ Tech Stack
Language: C++ (Standard 11+)

Frontend: HTML5 Canvas, JavaScript (for monitor.html).

Server: Python http.server (automates hosting the visualization).

OS: Windows (optimized for windows.h console features).

🎮 Usage Guide
Admin Credentials (Default)
Username: admin

Password: admin

Key Menu Options
[1] New Parcel & Dispatch: Create a shipment and assign it to a rider/route.

[6] Live Transit Monitor: Watch the status of parcels update in real-time in the console.

[8] Global Map View: See the text-based representation of the city network.

[9] Live Monitor (Web): Updates the JSON data for the browser visualization.
