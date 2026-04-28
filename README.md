# COMP3015-CW2

## Project Overview
This project is an OpenGL graphics prototype developed into a small playable **lane-runner style game**, themed around an **X-Wing flying over an animated ocean while avoiding incoming TIE fighters**.

Created as one of my final university graphics projects, the aim was not only to demonstrate individual rendering techniques, but to combine them into a more complete and interactive real-time experience. Rather than producing another static technical demo, I wanted this project to feel more like a small game prototype, with clear gameplay structure, visual feedback, and a stronger sense of presentation.

The project builds on the earlier coursework foundation, but its main focus for CW2 is on integrating more advanced graphics techniques into a single cohesive scene. The key CW2 features are:

- **Surface animation** for the ocean using layered procedural wave motion
- **Instanced particle effects** for the X-Wing engines
- **Explosion and smoke particle systems** for the game over sequence
- **Shadow mapping** for scene depth and stronger visual grounding
- **Gameplay systems** including a start screen, playing state, game over state, score system, increasing speed over time, obstacle spawning, and audio feedback
- **Custom bitmap UI text rendering** for menus and HUD text

The project also retains a number of foundational rendering features developed previously in CW1, including the skybox, textured model rendering, normal mapping, and Blinn–Phong lighting. These are still part of the final project and contribute to the overall presentation, but they are not the primary features being claimed as new CW2 work. Instead, they act as the visual and technical base that the newer CW2 systems are built upon.

The overall goal of the project is to present a more complete, portfolio-style graphics prototype rather than only a static rendering exercise, combining shader work, particles, animation, and game flow into one cohesive scene.

I chose to build the project around Star Wars-inspired assets and themes because they reflect my own interests and allowed me to make one of my final university projects feel more personal, while still remaining focused on the technical goals of the module.



https://github.com/user-attachments/assets/f8852f25-2116-4632-91d6-ff4701f15c19




---

## Download / Build / Run

### Download from GitHub (ZIP)
1. Open the repository page on GitHub.
2. Click **Code** → **Download ZIP**.
3. Extract the ZIP somewhere sensible, such as `Documents/COMP3015-CW2/`.

Repository link:  
https://github.com/gingeapple182/COMP3015-CW2


### Running the pre-built executable
- A compiled **.exe** is included in the submission/build files.
- Run the executable directly to use the project without opening Visual Studio.
- If Windows blocks it: right click → **Properties** → tick **Unblock** → **Apply**.

### Building from source (Visual Studio)
1. Open the solution file in Visual Studio.
2. Set configuration to **Release**.
3. Set platform to **x64** if supported by the provided template.
4. Build using **Build → Build Solution**.
5. Run with **Local Windows Debugger** or **Ctrl+F5**.

### Important note about assets / libraries
This project depends on included assets such as:
- `media/` textures, models, and audio
- shader files
- the **irrKlang** audio library

If anything appears missing when running from source, make sure the project working directory is set correctly so the executable can find the `media/` folder and required DLL/dependency files.

If needed, irrKlang can be obtained from:  
[https://www.ambiera.com/irrklang/](https://github.com/jonas2602/irrKlang/tree/master)

---

## How to Use (Controls)

### Menu / State Controls
- **F** — Start the game from the start screen
- **F** — Restart after game over

### Gameplay Controls
- **A / D** or **Left / Right Arrow Keys** — Move between lanes

### Gameplay Goal
- Avoid incoming **TIE fighters**
- Gain score by successfully dodging them
- Survive as long as possible while the game speed gradually increases

---

## Technical Summary (CW2 Features Implemented)

### 1. Surface Animation
The ocean surface is animated in `shader/wave.vert` using multiple layered sine waves with different amplitudes, frequencies, speeds, and directions. This creates a more natural-looking moving surface than a single repeating wave.

The wave system also scrolls relative to gameplay movement using a forward offset, helping the scene feel continuous while the player remains centred.


https://github.com/user-attachments/assets/9cc2afbb-3c35-4667-8a6e-05dbf4f12974


### 2. Shadow Mapping
The scene uses **shadow mapping** to add more depth and contact between objects and the environment.

A shadow framebuffer and depth texture are created in code, the scene is rendered from the light’s point of view, and the resulting shadow map is sampled in the main object shader and the wave shader. This helps ground the X-Wing, TIE fighters, and animated environment more effectively than lighting alone.


https://github.com/user-attachments/assets/a0b99337-d055-4b6b-89d4-9dfcf6dda63d


### 3. Engine Particle Effects
The X-Wing engines use an **instanced particle system** driven by `particle_fountain.vert` / `particle_fountain.frag`.

Particles are emitted from four engine positions and rendered as billboarded quads. Their motion is based on initial velocity, lifetime, gravity, and emitter position. These particles run continuously during active gameplay to reinforce the sense of movement and propulsion.

### 4. Explosion and Smoke Effects
When the player collides with a TIE fighter, the game enters a **game over** state and triggers:
- an **explosion particle burst**
- a **rising smoke particle effect**

The explosion uses a short, outward burst of particles, while the smoke uses a longer-lived system with softer upward motion and changing particle size over time. These two effects work together to create a more satisfying failure state than simply stopping play.


https://github.com/user-attachments/assets/2f4bd1fa-d864-44ae-8eb2-65c1f6002388


### 5. Gameplay Structure / Gamification
Rather than remaining only a rendering showcase, the project is structured as a small playable experience with:
- **StartScreen**, **Playing**, and **GameOver** states
- lane-based movement
- randomised TIE obstacle spawning
- optional second TIE spawns for extra pressure
- collision detection
- increasing movement speed over time
- a score reward for dodging obstacles

This gives the graphics systems a gameplay context and makes the scene function more like a simple arcade prototype than a passive technical demo.

### 6. Audio Integration
The project uses **irrKlang** for audio playback.

Audio is used for:
- looping X-Wing engine sound during gameplay
- TIE flyby sounds when obstacles pass the player
- explosion sound on collision / game over

This improves feedback and makes the scene feel more complete as a playable prototype.

### 7. Custom Bitmap UI Text
The HUD and menu text are rendered using a custom **bitmap text renderer** rather than default UI text.

This system uses:
- `shader/text.vert`
- `shader/text.frag`
- `text_renderer.h`
- `text_renderer.cpp`

It renders text from a bitmap atlas and supports shadowed text for improved readability on top of the 3D scene. It is used for the start screen, score display, and game over screen.
<p align="center">
  <img src="https://github.com/user-attachments/assets/50a9edb1-0eb6-4b0f-97b0-5b127dd00ac8" alt="spr_font_hellomyoldfriend_12x12_by_lotovik_sheet" height="264" />
  <img src="https://github.com/user-attachments/assets/4ac1a12a-7250-44b6-9a93-45442fcdd1fb" alt="image" height="264" />
</p>

---

## How the Program Code Fits Together

### Entry Point
- `main.cpp`
  - Creates the scene runner
  - Instantiates `SceneBasic_Uniform`
  - Starts the application loop

### Main Scene / Gameplay Logic
- `scenebasic_uniform.h`
- `scenebasic_uniform.cpp`

These contain the core project logic:
- scene setup
- shader compilation
- texture/model/audio loading
- render passes
- game state handling
- score and obstacle systems
- particle setup and updates
- camera behaviour
- UI rendering

### Main Scene Flow
Important scene methods include:
- `initScene()` — loads shaders, textures, audio, particle buffers, text renderer, and initial game state
- `update(float t)` — updates timing, input, gameplay state, difficulty, collision checks, particles, and camera
- `render()` — performs shadow pass, main scene rendering, wave rendering, particle rendering, and UI rendering

### Gameplay Helpers
Gameplay behaviour is separated into helper methods such as:
- `updateLaneInput()`
- `updateShipLanePosition()`
- `resetTieObstacle()`
- `updateTieObstacle()`
- `checkTieCollision()`
- `triggerGameOver()`
- `restartGame()`

These make it easier to follow the gameplay loop without all logic being embedded directly inside `update()`.

### Shaders
Main shader files used for CW2 include:

- `shader/basic_uniform.vert`
- `shader/basic_uniform.frag`
  - main object lighting / shadows

- `shader/wave.vert`
- `shader/wave.frag`
  - animated ocean surface and wave shading

- `shader/particle_fountain.vert`
- `shader/particle_fountain.frag`
  - engine and explosion particles

- `shader/particle_smoke.vert`
- `shader/particle_fountain.frag`
  - smoke particle system

- `shader/text.vert`
- `shader/text.frag`
  - bitmap UI text rendering

### Text Rendering
- `text_renderer.h`
- `text_renderer.cpp`

These files manage:
- loading the bitmap charset
- setting up the font texture and buffers
- generating per-character quads
- rendering normal or shadowed text to the screen

---


## Where to Find Things (Quick Navigation)

### Main application
- `main.cpp`

### Main scene / render + gameplay logic
- `scenebasic_uniform.h`
- `scenebasic_uniform.cpp`

### Surface animation
- `shader/wave.vert`
- `shader/wave.frag`

### Main object shader + shadows
- `shader/basic_uniform.vert`
- `shader/basic_uniform.frag`

### Engine / explosion particles
- `shader/particle_fountain.vert`
- `shader/particle_fountain.frag`

### Smoke particles
- `shader/particle_smoke.vert`
- `shader/particle_fountain.frag`

### Bitmap text rendering
- `shader/text.vert`
- `shader/text.frag`
- `text_renderer.h`
- `text_renderer.cpp`

---

## References

### Core framework / starter template
- Module-provided Visual Studio / OpenGL template from **COMP3015 Lab 1**.

### External assets
#### Models
- **X-Wing**  
  "Star Wars Galaxies - X-Wing(Luke/Red-Five)" by Zorg_Sinister  
  https://skfb.ly/pwpGG
  Licensed under **CC BY 4.0**

- **TIE Fighter**  
  "Star Wars Battlefront - TIE Fighter" by Neut2000  
  https://skfb.ly/oRtLV
  Licensed under **CC BY 4.0**

#### Skybox
- https://freestylized.com/skybox/sky_109/
  Royalty Free License: free to use for commercial and non-commercial purposes

#### Font
- https://lotovik.itch.io/hello-my-old-friend
  **bitmap font - hello, my old friend © 2022 by lotovik**  
  Licensed under **CC BY-SA 4.0**

#### Audio
- `xwing_loop`  
  https://freesound.org/people/raelwissdorf/sounds/99971/

- `X-Wing flyby 2`  
  https://www.soundboard.com/sb/sound/963779

- `X-Wing flyby 6`  
  https://www.soundboard.com/sb/sound/963783

- `X-Wing explode`  
  https://www.soundboard.com/sb/sound/963774

- `TIE fighter flyby 1`  
  https://www.soundboard.com/sb/sound/963768

### Audio licensing note
Some audio taken from `soundboard.com` is listed as **personal-use** content and would not normally be appropriate for commercial release. For this coursework submission, I have lecturer permission to use these assets within the limits of the educational assessment. If this project were ever developed further for public or commercial release, those sounds would need to be replaced with appropriately licensed alternatives.

### Other provided resources
- Some supporting assets/resources are taken from the **module lab materials provided by the university**

---

## YouTube Video Link
- **YouTube video link:**  
  https://youtu.be/t_OsZX-mWiY
---

## AI Declaration (Generative AI Use)

### Declared level (per brief)
This coursework used Generative AI at the **Partnered Work (P1)** level, consistent with the brief’s permitted uses, which include code assistance, programming testing, and README/report crafting. :contentReference[oaicite:4]{index=4} :contentReference[oaicite:5]{index=5}

### What GenAI was used for
Generative AI was used to:
- discuss rendering/shader implementation ideas
- help debug OpenGL, shader, and state-management issues
- support implementation planning for particles, shadows, gameplay logic, and UI text
- help structure the README and related written documentation

### What remained my own work
- final implementation and integration into the COMP3015 template framework
- all design and technical decisions used in the final submitted project
- scene composition, asset choice, tuning, balancing, and testing
- final video content and project explanation

### AI prompts / transcripts
Per the brief requirement that all features must be accompanied by AI prompts/transcripts, relevant transcripts are included alongside the submission materials and organised by feature.

### Signed form
A signed Generative AI Declaration form is included in the final submission materials as required.

https://github.com/user-attachments/assets/a453e2fb-9c5c-4cc1-9de9-5500ec4fedfc

