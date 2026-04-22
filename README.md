# Procedural World Simulator & Map Engine (UE5 / C++)

A custom procedural generation tool built in **Unreal Engine 5** using **C++**, designed to explore algorithmic world-building, geographic noise layering, and data visualization. 

<div align="center">
  <img width="850" alt="Map Image" src="https://github.com/user-attachments/assets/3a9e739e-9641-4831-bc3a-3a6cf9dd8d27"/>
</div>

<br>

**This project is a work in progress (and will probably be for a while)!**

## Progress so far

* **Procedural Terrain Generation (C++):** Uses layered Perlin Noise (Octaves and Ridged Noise) combined with falloff to generate realistic landmasses and islands. 
* **Political Expansion Simulation:** Generates kingdom borders using a breadth-first search / flood-fill algorithm. High-elevation points act as natural barriers/weights to emulate the influence geographical features have on country borders.
* **Map Rendering:** Built a two layer rendering system that calculates map data on the CPU and pushes it directly to GPU texture memory using FTexture2DMipMap locking and UpdateTextureRegions, allowing for instant map updates without frame drops when using map customization tools.

<br>

<div align="center">
  <img width="500" alt="Political View" src="https://github.com/user-attachments/assets/76a7af5e-3141-4590-a516-0d18a03a4222" /> <br>
  <i>Political View</i>
</div>

<br>

<div align="center">
  <img width="500" alt="Terrain/Physical View" src="https://github.com/user-attachments/assets/7cbdfaa6-5aea-484d-aa8f-aa1accc5288e" /> <br>
  <i>Terrain Edit View</i>
</div>


<br>

* **Terrain Paint Tool:** An interactive brush that allows for gradual terrain alteration. The brush works in either an increasing or decreasing way, allowing users to lift terrain or carve it down to sea level.


<div align="center">
  <img width="500" alt="Terrain/Physical View" src="https://github.com/user-attachments/assets/dfde3f0b-7a23-4b37-90e1-a8ba9b6a6870" /> <br>
  <i>Terrain Brush Tool</i>
</div>

<br>

## Future Additions

### Near-Term Objectives
* **Labeling System:** Implement an algorithm to place city markers and territory labels evenly without overlapping UI elements.
* **Algorithm Refinement:** Refine the brush tool logic to blend painted terrain seamlessly into the procedural noise. Also improve the border generation to look as natural as possible.
* **Kingdom Attribute & Border Customization**: Allow the user to change each kingdom's attributes (e.g. name, colour, population, etc.) using interactive UI, and borders with a Political Layer paint tool.
* **Lasso Tools:** Create a tool that lets the user select a specific area of the map, alter its attributes in the current layer, and regenerate it. 
* **River Generation:** Use pathfinding algorithms to simulate water flow from high elevation points down to sea level.
* **Cities and Places of Interest:** Add random but realistic generation of settlements, with the ability to relocate them, change attributes, and manually add custom ones.

### Far-Future Features
* **Additional Layers:** Adding more generation layers (Climate, Religions, Peoples, Languages, etc.) to support many-faceted worldbuilding.
* **Improved Graphics & 3D:** While the current focus is 2D data visualization, I intend to add the option to view the generated heightmaps and biomes in full 3D space.
* **Improved UI:** Refining the look, consistency, and flow of the UMG elements.
* **Data Export:** Adding functionality to export the generated world data as JSON or texture maps, as is or with custom 2D themes.
