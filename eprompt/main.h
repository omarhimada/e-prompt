#pragma once

#include "Resource.h"

#define MAX_LOADSTRING 100
#define BUTTON_HEIGHT 28

#define ID_INPUT_EDIT 1001
#define ID_SORT_PROMPT_BUTTON 1002
#define ID_COPY_OUTPUT_BUTTON 1003
#define ID_OUTPUT_EDIT 1004
#define ID_FONT_CUSTOM 1005

// Global Variables:
HINSTANCE hInst;                                // Current instance
WCHAR szTitle[MAX_LOADSTRING];                  // Title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // Main window class name
HWND hInputEdit;                                // Input
HWND hSortPromptButton;                         // Sort button
HWND hCopyOutputButton;							// Copy button
HWND hOutputDisplay;                            // Output
WNDPROC g_DefaultEditProc;						// 'CTRL+A' selection
HBRUSH g_hTextAreaBgBrush;						// I/O background brush
HFONT hFont;									// Font

// Forward declarations of functions included in this code module:
ATOM                RegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                ResizeControls(HWND hWnd);
void                SortPrompts();

struct Prompt {
	std::wstring original;
	std::wstring text;
	double weight;
};

// Loads an example negative prompt for testing
void LoadExampleNegativePrompt() {
	const wchar_t* exampleText =
		L"example negative prompts, mud, cave, stump, dog, canyon, hood, (ink:1.2), ugly, superhero, denim, valley, flea, crow, (sketch), muddy, bat, nature, wall outlet, abstract, cricket, creature, raccoon, football, old-concrete-floor, spider, ditch, branch, disgusting, garbage, cesspool, snake, dump, leather, mountain, zombie, lion, flower, beard, woodland, soil, devil, cave, moss, trash, prison, piranha, Elmo, puddle, grotesque, lizard, bee, hell, turtle, hornet, anime, bad anatomy, diaper, lake, leaves, pit, pen, pond, mold, pig, wrestler, creepy, ISS, branch, dusty, deer, cobra, bulky, ash, football helmet, bog, rotten, haptic, basketball, tree, otter, meadow, graveyard, puddle, comic, turtle shell, camel, football player, tarantula, signature, oil, filthy, dragonfly, pencil, quagmire, muddy boots, beast, root, louse, forest, grasshopper, manticore, sludge, abstract, devil horns, sandy, mosquito, cricket, paint, gold dust, hawk, foliage, ape, whale, zombie, fish, creepy doll, sand, branch, gnat, bug, rotten meat, marsh, moldy, wasp, grassland, charred, otter, muddy water, rubble, tick, tree roots, muddy ground, bulldozer, trench, trash bag, sketchbook, bat wings, lizard scales, sludge pit, tar, football jersey, clay, cap, muddy road, leather jacket, puddle of mud, wall, swamp, dragon, salamander, cave entrance, wrestler mask, old wood floor, toad, toadstool, gill, house-centipede, horror, rotten tree, gravel, hooded figure, mantis, goblin, spider web, stone, portrait, creature design, helmet, phone, millipede, football field, dust, sketch lines, trash pile, comic book, drawing, cave wall, landscape, football stadium, mud puddle, slime, paper, superhero cape, landfill, terrain, valley floor, roots, goat, shark, football cleats, beastly, puddles, crow feathers, crypt, clay pot, football gloves, desolate, rotten leaves, mildew, sketch art, muddy trail, filthy water, mossy rock, horror comic, garbage dump, swamp water, ugly anatomy, bark, smoke, soot, spider legs, lizard tail, bat cave, insect, muddy swamp, rotten swamp, creepy forest, dirty jeans, rotten wood, branch pile, venom, viper, sludge monster, cave floor, rough sketch, skeleton, monstrous, rotten bog, muddy ditch, wasteland, rubble pile, stone wall, rotten grass, rotten soil, rotten roots, wasteland terrain, bog water, sewage, maggot, lice, leech, skeleton, coffin, ghoul, demon, ogre, troll, gremlin, monster, [[[alien]]], kraken, octopus, squid, alligator, crocodile, porcupine, walrus, seal, parrot, giraffe, horse, monkey, baboon, bear, cougar, reptile, dragonfly, earwig, beetle, cockroach, roach, cicada, pest, superbug, crawler, bed bugs, (((illustration))), ((painting)), ((cartoon)), (toon), stencil, charcoal, crayon, analog, tile, zipper, button, tool, [table], bed, pillow, curtain, door, cage, car, phone, wood floor, hotdog, meatball, sherbet, matches";

	SetWindowTextW(hInputEdit, exampleText);
}