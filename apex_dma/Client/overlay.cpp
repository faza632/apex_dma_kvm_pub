#include "overlay.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <X11/Xatom.h>

#include <fstream>
#include <functional>
#include <iomanip>
#include <thread>

using namespace std;

extern float veltest;
extern settings_t global_settings;
extern int local_held_id;
extern uint32_t local_weapon_id;
extern bool ready;
extern float bulletspeed;
extern float bulletgrav;

// Aimbot
extern bool lock;          // read lock state
extern bool aimbot_safety; // read safety state
extern float max_fov;
extern int spectators;
extern int allied_spectators;
// Left and Right Aim key toggle
bool toggleaim = false;
bool toggleaim2 = false;
int e = 0;
// Main Map Radar
bool mainradartoggle = 1;

// Menu Stuff
int menu1 = 0;
int menu2 = 0;
int menu3 = 0;
int menu4 = 0;
// // screen pos ajuster
// // ajuster for screen pos
// extern int worldsedgetoprightx = 0;
// extern int worldsedgetoprighty = 0;
// extern int worldsedgebtmleftx = 0;
// extern int worldsedgebtmlefty = 0;

int width;
int height;
bool k_leftclick = false;
bool k_ins = false;
bool show_menu = false;

// extern bool IsKeyDown(int vk);
extern bool IsKeyDown(ImGuiKey imgui_k);
extern bool isPressed(uint32_t button_code);

static void StaticMessageStart(Overlay &ov) { ov.CreateOverlay(); }

void Overlay::RenderMenu() {
  static bool aim_enable = false;
  static bool vis_check = false;

  if (global_settings.aim > 0) {
    aim_enable = true;
    if (global_settings.aim > 1) {
      vis_check = true;
    } else {
      vis_check = false;
    }
  } else {
    aim_enable = false;
    vis_check = false;
  }

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(450, 860), ImGuiCond_Once);
  ImGui::Begin(XorStr("##title"), (bool *)true,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
  // if (ImGui::BeginTabBar(XorStr("Tab")))
  //{
  // if (ImGui::BeginTabItem(XorStr("##")))
  //{
  if (ImGui::CollapsingHeader("Main Toggle Settings")) {
    menu1 = 1;
    ImGui::Checkbox(XorStr("ESP On/Off"), &global_settings.esp);
    // ImGui::SameLine();
    // ImGui::Checkbox(XorStr("Thirdperson"), &thirdperson);

    ImGui::Checkbox(XorStr("Glow Items"), &global_settings.item_glow);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Mini-Map Radar"), &global_settings.mini_map_radar);

    ImGui::Checkbox(XorStr("Glow Players"), &global_settings.player_glow);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Mini-Map Guide"), &global_settings.mini_map_guides);

    ImGui::Checkbox(XorStr("AIM On/Off"), &aim_enable);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("1v1"), &global_settings.onevone);

    if (aim_enable) {
      ImGui::Checkbox(XorStr("Visibility Check"), &vis_check);
      ImGui::SameLine();
      ImGui::Checkbox(XorStr("No Recoil"), &global_settings.aim_no_recoil);
      ImGui::SameLine();
      ImGui::Checkbox(XorStr("No Nade Aim"), &global_settings.no_nade_aim);
      if (vis_check) {
        global_settings.aim = 2;
      } else {
        global_settings.aim = 1;
      }
    } else {
      global_settings.aim = 0;
    }

    ImGui::Checkbox(XorStr("Firing Range"), &global_settings.firing_range);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("TDM Toggle"), &global_settings.tdm_toggle);
    ImGui::Checkbox(XorStr("Press F8 enable MapRadar"),
                    &global_settings.map_radar_testing);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Aiming Distance:"));
    ImGui::SameLine();
    ImGui::TextColored(GREEN, "%.f meters", global_settings.aim_dist / 39.62);
    ImGui::SliderFloat(XorStr("##Aim Distance"), &global_settings.aim_dist,
                       10.0f * 39.62, 1600.0f * 39.62, "##");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Aiming Keys:"));
    ImGui::RadioButton("Left Mouse", &e, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Right Mouse ", &e, 2);
    ImGui::SameLine();
    ImGui::RadioButton("Left/Right Mouse", &e, 3);
    // Setting one and unsetting the other
    if (e == 1) {
      toggleaim = true;
      toggleaim2 = false;
    } else if (e == 2) {
      toggleaim = false;
      toggleaim2 = true;
    } else if (e == 3) {
      toggleaim = true;
      toggleaim2 = true;
    }
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Max distance for everything:"));
    ImGui::SameLine();
    ImGui::TextColored(GREEN, "%d meters",
                       (int)(global_settings.max_dist / 40));
    ImGui::SliderFloat(XorStr("##1"), &global_settings.max_dist, 100.0f * 40,
                       3800.0f * 40, "##");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImGui::Text(XorStr("Max fov:"));
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::Text(XorStr("non-ADS:"));
    ImGui::SameLine();
    ImGui::TextColored(GREEN, "%.f", global_settings.non_ads_fov);
    ImGui::SliderFloat(XorStr("##nonADSfov"), &global_settings.non_ads_fov,
                       5.0f, 50.0f, "##");
    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    ImGui::Text(XorStr("ADS:"));
    ImGui::SameLine();
    ImGui::TextColored(GREEN, "%.f", global_settings.ads_fov);
    ImGui::SliderFloat(XorStr("##ADSfov"), &global_settings.ads_fov, 5.0f,
                       50.0f, "##");
    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    ImGui::Text(XorStr("Current:"));
    ImGui::SameLine();
    ImGui::TextColored(GREEN, "%.f", max_fov);
    ImGui::SliderFloat(XorStr("##max_fov"), &max_fov, 5.0f, 50.0f, "##");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    ImGui::Text(XorStr("Smooth Aim Value:"));
    ImGui::SameLine();
    if (global_settings.smooth < 100.0f) {
      ImGui::TextColored(RED, "%.f", global_settings.smooth);
    } else if (global_settings.smooth > 120.0f) {
      ImGui::TextColored(GREEN, "%.f", global_settings.smooth);
    } else {
      ImGui::TextColored(WHITE, "%.f", global_settings.smooth);
    }
    ImGui::SliderFloat(XorStr("##2"), &global_settings.smooth, 50.0f, 250.0f,
                       "##");
    ImGui::SameLine();
    ImGui::Text(XorStr("85 To 150 Is Safe"));
    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    ImGui::Text(XorStr("Smooth Skynade Aim:"));
    ImGui::SameLine();
    global_settings.skynade_smooth = global_settings.smooth * 0.6667f;
    ImGui::TextColored(WHITE, "%.f", global_settings.skynade_smooth);

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImGui::Text(XorStr("Smooth Preditcion Speed:"));
    ImGui::SameLine();
    ImGui::TextColored(GREEN, "%.2f", bulletspeed);
    ImGui::SliderFloat(XorStr("##55"), &bulletspeed, -10.58f, 5.80f, "##");
    ImGui::SameLine();
    ImGui::Text(XorStr("Default is 0.08"));
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImGui::Text(XorStr("Smooth Preditcion Gravity:"));
    ImGui::SameLine();
    ImGui::TextColored(GREEN, "%.2f", bulletgrav);
    ImGui::SliderFloat(XorStr("##57"), &bulletgrav, -10.55f, 5.90f, "##");
    ImGui::SameLine();
    ImGui::Text(XorStr("Default is 0.05"));
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Aiming Bone:"));
    ImGui::Checkbox(XorStr("Auto"), &global_settings.bone_auto);
    ImGui::Text(XorStr("0=Head, 1=Neck, 2=Chest, 3=Stomach"));
    ImGui::SliderInt(XorStr("##bone"), &global_settings.bone, 0, 3);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("ESP Options:"));
    ImGui::Checkbox(XorStr("Box"), &global_settings.esp_visuals.box);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Line"), &global_settings.esp_visuals.line);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Distance"), &global_settings.esp_visuals.distance);
    ImGui::Checkbox(XorStr("Health bar"),
                    &global_settings.esp_visuals.healthbar);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Shield bar"),
                    &global_settings.esp_visuals.shieldbar);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Name"), &global_settings.esp_visuals.name);
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::Checkbox(XorStr("Show aimbot target"),
                    &global_settings.show_aim_target);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Player Glow Visable:"));
    float glowcolorviz[] = {global_settings.glow_r_viz,
                            global_settings.glow_g_viz,
                            global_settings.glow_b_viz};
    ImGui::ColorEdit3("##Glow Color Picker Visable", glowcolorviz);
    {
      global_settings.glow_r_viz = glowcolorviz[0];
      global_settings.glow_g_viz = glowcolorviz[1];
      global_settings.glow_b_viz = glowcolorviz[2];
    }
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Player Glow Not Visable:"));
    float glowcolornot[] = {global_settings.glow_r_not,
                            global_settings.glow_g_not,
                            global_settings.glow_b_not};
    ImGui::ColorEdit3("##Glow Color Not Visable", glowcolornot);
    {
      global_settings.glow_r_not = glowcolornot[0];
      global_settings.glow_g_not = glowcolornot[1];
      global_settings.glow_b_not = glowcolornot[2];
    }
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Player Glow Knocked:"));
    float glowcolorknocked[] = {global_settings.glow_r_knocked,
                                global_settings.glow_g_knocked,
                                global_settings.glow_b_knocked};
    ImGui::ColorEdit3("##Glow Color Knocked", glowcolorknocked);
    {
      global_settings.glow_r_knocked = glowcolorknocked[0];
      global_settings.glow_g_knocked = glowcolorknocked[1];
      global_settings.glow_b_knocked = glowcolorknocked[2];
    }
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(
        XorStr("Saving and Loading. Need to Save Once to make the file."));
    // Saving
    if (ImGui::Button("Save Config")) {
      save_settings(global_settings);
    }
    ImGui::SameLine();
    // Loading
    if (ImGui::Button("Load Config")) {
      global_settings = load_settings();
    }
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    if (menu1 == 1) {
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Weapone Filter Settings"),
                                       0);
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Item Filter Settings"), 0);
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Radar Settings"), 0);
    }
  }
  if (ImGui::CollapsingHeader("Radar Settings")) {
    menu2 = 1;
    // Dot Size for both mini and main map
    ImGui::Text(XorStr("MiniMap Radar Dot Size"));
    ImGui::SliderInt(XorStr("MiniMap Dot Size"),
                     &global_settings.mini_map_radar_dot_size1, 1, 10);
    ImGui::SliderInt(XorStr("MiniMap Outer Ring Thickness"),
                     &global_settings.mini_map_radar_dot_size2, 1, 10);
    ImGui::Text(XorStr("Main Map Radar Dot Size"));
    ImGui::SliderInt(XorStr("Main Map Dot Width"),
                     &global_settings.main_map_radar_dot_size1, 1, 10);
    ImGui::SliderInt(XorStr("Main Map Dot length"),
                     &global_settings.main_map_radar_dot_size2, 1, 10);
    /*//Radar Color
    ImGui::Text(XorStr("Radar Color Picker:"));
    ImGui::ColorEdit3("##Radar Color Picker", radarcolor);
    {
            radarcolorr = radarcolor[0];
            radarcolorg = radarcolor[1];
            radarcolorb = radarcolor[2];
    }
    */
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    if (menu2 == 1) {
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Weapone Filter Settings"),
                                       0);
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Item Filter Settings"), 0);
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Main Toggle Settings"), 0);
    }
  }
  if (ImGui::CollapsingHeader("Item Filter Settings")) {
    menu3 = 1;
    ImGui::Text(XorStr("Ammo"));
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::Checkbox(XorStr("Sniper Ammo"), &global_settings.loot_sniperammo);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Heavy Ammo"), &global_settings.loot_heavyammo);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Light Ammo"), &global_settings.loot_lightammo);
    ImGui::Checkbox(XorStr("Energy Ammo"), &global_settings.loot_energyammo);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Shotgun Ammo"), &global_settings.loot_shotgunammo);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Ammo Mags"));
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::Checkbox(XorStr("Sniper lv3"), &global_settings.loot_sniperammomag3);
    ImGui::Checkbox(XorStr("Sniper lv4"), &global_settings.loot_sniperammomag4);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Heavy lv3"), &global_settings.loot_heavyammomag3);
    ImGui::Checkbox(XorStr("Heavy lv4"), &global_settings.loot_heavyammomag4);
    ImGui::Checkbox(XorStr("Light lv3"), &global_settings.loot_lightammomag3);
    ImGui::Checkbox(XorStr("Light lv4"), &global_settings.loot_lightammomag4);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Energy lv3"), &global_settings.loot_energyammomag3);
    ImGui::Checkbox(XorStr("Energy lv4"), &global_settings.loot_energyammomag4);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("HCOGs"));
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::Checkbox(XorStr("1x HCOG"), &global_settings.loot_optic1xhcog);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("2x HCOG"), &global_settings.loot_optic2xhcog);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("3x HCOG"), &global_settings.loot_optic3xhcog);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("2-4x"), &global_settings.loot_optic2x4x);
    ImGui::Text(XorStr("Snipers"));
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::Checkbox(XorStr("Sniper 6x"), &global_settings.loot_opticsniper6x);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Sniper 4-8x"),
                    &global_settings.loot_opticsniper4x8x);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Sniper Threat"),
                    &global_settings.loot_opticsniperthreat);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Holo's"));
    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::Checkbox(XorStr("1x Holo"), &global_settings.loot_opticholo1x);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("2x Holo"), &global_settings.loot_opticholo1x2x);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("1x Threat"), &global_settings.loot_opticthreat);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Backpacks"));
    ImGui::Checkbox(XorStr("Light Backpack"),
                    &global_settings.loot_lightbackpack);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Medium Backpack"),
                    &global_settings.loot_medbackpack);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Heavy Backpack"),
                    &global_settings.loot_heavybackpack);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Shield's"));
    ImGui::Checkbox(XorStr("Armor blue"), &global_settings.loot_shieldupgrade2);
    ImGui::Checkbox(XorStr("Armor purple"),
                    &global_settings.loot_shieldupgrade3);
    ImGui::Checkbox(XorStr("Armor gold"), &global_settings.loot_shieldupgrade4);
    ImGui::Checkbox(XorStr("Armor red"), &global_settings.loot_shieldupgrade5);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Helmets blue"),
                    &global_settings.loot_shieldupgradehead2);
    ImGui::Checkbox(XorStr("Helmets purple"),
                    &global_settings.loot_shieldupgradehead3);
    ImGui::Checkbox(XorStr("Helmets gold"),
                    &global_settings.loot_shieldupgradehead4);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Knockdown blue"),
                    &global_settings.loot_shielddown2);
    ImGui::Checkbox(XorStr("Knockdown purple"),
                    &global_settings.loot_shielddown3);
    ImGui::Checkbox(XorStr("Knockdown gold"),
                    &global_settings.loot_shielddown4);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextColored(GREEN, "Heals for Health");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Checkbox(XorStr("Large Health"), &global_settings.loot_healthlarge);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Small Health"), &global_settings.loot_healthsmall);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Phoenix Kit"), &global_settings.loot_phoenix);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextColored(BLUE, "Heals for Shields");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Checkbox(XorStr("Large Shield"),
                    &global_settings.loot_shieldbattlarge);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Small Shield"),
                    &global_settings.loot_shieldbattsmall);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Ultimate Accelerant"),
                    &global_settings.loot_accelerant);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Attachements"));
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::Checkbox(XorStr("Lasersight2"), &global_settings.loot_lasersight2);
    ImGui::Checkbox(XorStr("Lasersight3"), &global_settings.loot_lasersight3);
    ImGui::Checkbox(XorStr("Lasersight4"), &global_settings.loot_lasersight4);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Sniper Stock 2"),
                    &global_settings.loot_stocksniper2);
    ImGui::Checkbox(XorStr("Sniper Stock 3"),
                    &global_settings.loot_stocksniper3);
    ImGui::Checkbox(XorStr("Sniper Stock 4"),
                    &global_settings.loot_stocksniper4);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Regular Stock 2"),
                    &global_settings.loot_stockregular2);
    ImGui::Checkbox(XorStr("Regular Stock 3"),
                    &global_settings.loot_stockregular3);
    ImGui::Checkbox(XorStr("Suppressor 1"), &global_settings.loot_suppressor1);
    ImGui::Checkbox(XorStr("Suppressor 2"), &global_settings.loot_suppressor2);
    ImGui::Checkbox(XorStr("Suppressor 3"), &global_settings.loot_suppressor3);
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::Text(XorStr("Weapon Mods"));
    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::Checkbox(XorStr("Turbo Charger"),
                    &global_settings.loot_turbo_charger);
    ImGui::Checkbox(XorStr("Skull Piecer"), &global_settings.loot_skull_piecer);
    ImGui::Checkbox(XorStr("Hammer Point"), &global_settings.loot_hammer_point);
    ImGui::Checkbox(XorStr("Disruptor Rounds"),
                    &global_settings.loot_disruptor_rounds);
    ImGui::Checkbox(XorStr("Boosted Loader"),
                    &global_settings.loot_boosted_loader);
    ImGui::Checkbox(XorStr("Shotgunbolt 1"),
                    &global_settings.loot_shotgunbolt1);
    ImGui::Checkbox(XorStr("Shotgunbolt 2"),
                    &global_settings.loot_shotgunbolt2);
    ImGui::Checkbox(XorStr("Shotgunbolt 3"),
                    &global_settings.loot_shotgunbolt3);
    ImGui::Checkbox(XorStr("Shotgunbolt 4"),
                    &global_settings.loot_shotgunbolt4);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    {
      ImGui::Text(XorStr("Loot Glow Filled:"));
      int lootfilled_value = global_settings.loot_filled;
      ImGui::SliderInt(XorStr("##lootfilled"), &lootfilled_value, 0, 14, "%d");
      global_settings.loot_filled = lootfilled_value;
    }
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    if (menu3 == 1) {
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Weapone Filter Settings"),
                                       0);
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Radar Settings"), 0);
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Main Toggle Settings"), 0);
    }
  }
  if (ImGui::CollapsingHeader("Weapone Filter Settings")) {
    menu4 = 1;
    // Light Weapons
    ImGui::TextColored(ORANGE, "Light Weapons");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Checkbox(XorStr("P2020"), &global_settings.loot_weapon_p2020);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("RE-45"), &global_settings.loot_weapon_re45);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("R-99"), &global_settings.loot_weapon_r99);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("G7 Scout"), &global_settings.loot_weapon_g7_scout);
    ImGui::Checkbox(XorStr("Spitfire"), &global_settings.loot_weapon_spitfire);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("R-301"), &global_settings.loot_weapon_r301);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Alternator "),
                    &global_settings.loot_weapon_alternator);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    // Heavy Weapons
    ImGui::TextColored(TEAL, "Heavy Weapons");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Checkbox(XorStr("C.A.R."), &global_settings.loot_weapon_car_smg);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Flatline"), &global_settings.loot_weapon_flatline);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Hemlok"), &global_settings.loot_weapon_hemlock);
    ImGui::Checkbox(XorStr("Prowler "), &global_settings.loot_weapon_prowler);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("30-30"),
                    &global_settings.loot_weapon_3030_repeater);
    ImGui::Checkbox(XorStr("Rampage"), &global_settings.loot_weapon_rampage);
    // Energy Weapons
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextColored(YELLOW, "Energy Weapons");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Checkbox(XorStr("Triple Take"),
                    &global_settings.loot_weapon_triple_take);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("L-STAR"), &global_settings.loot_weapon_lstar);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Volt"), &global_settings.loot_weapon_volt);
    ImGui::Checkbox(XorStr("Devotion "), &global_settings.loot_weapon_devotion);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("HAVOC"), &global_settings.loot_weapon_havoc);
    ImGui::Checkbox(XorStr("Nemesis"), &global_settings.loot_weapon_nemesis);

    // Shotgun Weapons
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextColored(RED, "Shotgun Weapons");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Checkbox(XorStr("EVA-8"), &global_settings.loot_weapon_eva8);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Mozambique"),
                    &global_settings.loot_weapon_mozambique);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Peacekeeper"),
                    &global_settings.loot_weapon_peacekeeper);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Mastiff"), &global_settings.loot_weapon_mastiff);
    // Sniper Weapons
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextColored(BLUE, "Sniper Weapons");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Checkbox(XorStr("Charge Rifle"),
                    &global_settings.loot_weapon_charge_rifle);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Longbow"), &global_settings.loot_weapon_longbow);
    ImGui::SameLine();
    ImGui::Checkbox(XorStr("Sentinel"), &global_settings.loot_weapon_sentinel);
    ImGui::Checkbox(XorStr("Wingman "), &global_settings.loot_weapon_wingman);
    // KRABER
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Text(XorStr("Special Weapons"));
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Checkbox(XorStr("Kraber .50-Cal Sniper"),
                    &global_settings.loot_weapon_kraber);
    ImGui::Checkbox(XorStr("Bocek Bow"), &global_settings.loot_weapon_bow);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    if (menu4 == 1) {
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Item Filter Settings"), 0);
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Radar Settings"), 0);
      ImGui::GetStateStorage()->SetInt(ImGui::GetID("Main Toggle Settings"), 0);
    }
  }
  // ImGui::EndTabItem();

  //}

  // ImGui::EndTabBar();
  //}
  ImGui::Dummy(ImVec2(0.0f, 10.0f));
  ImGui::Text(XorStr("held=%d, weapon=%d"), local_held_id, local_weapon_id);
  ImGui::Dummy(ImVec2(0.0f, 5.0f));
  ImGui::Text(XorStr("Overlay FPS: %.3f ms/frame (%.1f FPS)"),
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::Dummy(ImVec2(0.0f, 5.0f));

  ImGui::Text(XorStr("Game FPS for Aim Prediction:"));
  ImGui::SameLine();
  ImGui::Checkbox(XorStr("Calc Game FPS"), &global_settings.calc_game_fps);
  ImGui::SliderFloat(XorStr("##gamefps"), &global_settings.game_fps, 1.0f,
                     300.0f, "%.1f");

  ImGui::Dummy(ImVec2(0.0f, 5.0f));
  ImGui::Text(XorStr("external-overlay test build"));
  ImGui::End();
}

void Overlay::RenderInfo() {
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(280, 30));
  ImGui::Begin(XorStr("##info"), (bool *)true,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoScrollbar);
  DrawLine(ImVec2(1, 2), ImVec2(280, 2), RED, 2);
  if (spectators == 0) {
    ImGui::TextColored(GREEN, "%d", spectators);
  } else {
    ImGui::TextColored(RED, "%d", spectators);
  }
  ImGui::SameLine();
  ImGui::Text("--");
  ImGui::SameLine();
  ImGui::TextColored(GREEN, "%d", allied_spectators);
  ImGui::SameLine();
  ImGui::Text("--");
  ImGui::SameLine();
  ImGui::TextColored(WHITE, "%.f", max_fov);
  ImGui::SameLine();
  ImGui::Text("--");
  ImGui::SameLine();
  // Aim is on = 2, On but No Vis Check = 1, Off = 0
  if (lock) {
    ImGui::TextColored(aimbot_safety ? GREEN : ORANGE, "[TARGET LOCK!]");
  } else if (local_held_id == -251) {
    ImGui::TextColored(BLUE, "Skynade On");
  } else if (global_settings.aim == 2) {
    ImGui::TextColored(GREEN, "Aim On");

  } else if (global_settings.aim == 0) {
    ImGui::TextColored(RED, "Aim Off");
  } else {
    ImGui::TextColored(RED, "Aim On %d", global_settings.aim);
  }
  ImGui::SameLine();
  // if (triggerbot) {
  //   ImGui::TextColored(GREEN, "1v1 On");
  // } else {
  //   ImGui::TextColored(RED, "1v1 Off");
  // }
  DrawLine(ImVec2(1, 28), ImVec2(280, 28), RED, 2);
  ImGui::End();
}

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int Overlay::CreateOverlay() {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif
  glfwWindowHint(GLFW_RESIZABLE, 1);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(1920, 1080, "Client ImGui GLFW+OpenGL3",
                                        glfwGetPrimaryMonitor(), nullptr);
  if (window == nullptr)
    return 1;
  static const char *GamescopeOverlayProperty = "GAMESCOPE_EXTERNAL_OVERLAY";
  Display *x11_display = glfwGetX11Display();
  Window x11_window = glfwGetX11Window(window);
  if (x11_window && x11_display) {
    // Set atom for gamescope to render as an overlay.
    Atom overlay_atom =
        XInternAtom(x11_display, GamescopeOverlayProperty, False);
    uint32_t value = 1;
    XChangeProperty(x11_display, x11_window, overlay_atom, XA_CARDINAL, 32,
                    PropertyNewValue, (unsigned char *)&value, 1);
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

#include "impl/render/font.h"
  io.Fonts->AddFontFromMemoryCompressedTTF(
      LXGWNeoXiHei_compressed_data, LXGWNeoXiHei_compressed_size, 15, NULL,
      io.Fonts->GetGlyphRangesChineseFull());
  io.Fonts->Build();

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Our state
  bool show_demo_window = false;
  bool show_another_window = false;
  running = true;

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    if (!running)
      break;
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application, or clear/overwrite your copy of the
    // keyboard data. Generally you may always pass all inputs to dear imgui,
    // and hide them from your application based on those two flags.
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in
    // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
    // ImGui!).
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair
    // to create a named window.
    {
      ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                     // and append into it.

      ImGui::Checkbox(
          "Demo Window",
          &show_demo_window); // Edit bools storing our window open/close state

      ImGui::Text("Overlay average %.3f ms/frame (%.1f FPS)",
                  1000.0f / io.Framerate, io.Framerate);

      if (global_settings.calc_game_fps) {
        ImGui::Text("Game average %.3f ms/frame (%.1f FPS)",
                    1000.0f / global_settings.game_fps,
                    global_settings.game_fps);
      }

      ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window) {
      ImGui::Begin(
          "Another Window",
          &show_another_window); // Pass a pointer to our bool variable (the
                                 // window will have a closing button that will
                                 // clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me"))
        show_another_window = false;
      ImGui::End();
    }

    // Draw Main GUI
    // if (IsKeyDown(ImGuiKey_MouseLeft) && !k_leftclick) {
    //   io.MouseDown[0] = true;
    //   k_leftclick = true;
    // } else if (!IsKeyDown(ImGuiKey_MouseLeft) && k_leftclick) {
    //   io.MouseDown[0] = false;
    //   k_leftclick = false;
    // }
    {
      bool key_insert_pressed = IsKeyDown(ImGuiKey_Insert) || isPressed(72);
      if (key_insert_pressed && !k_ins && ready) {
        show_menu = !show_menu;
        k_ins = true;
      } else if (!key_insert_pressed && k_ins) {
        k_ins = false;
      }
    }

    // Main Map Radar, Needs Manual Setting of cords
    {
      bool key_m_pressed = IsKeyDown(ImGuiKey_M) || isPressed(23);
      if (key_m_pressed && mainradartoggle == 0) {
        mainradartoggle = 1;
        if (!global_settings.main_radar_map) {
          global_settings.main_radar_map = true;
          global_settings.mini_map_radar = false;
        } else {
          global_settings.main_radar_map = false;
          global_settings.mini_map_radar = true;
        }
      } else if (!key_m_pressed && mainradartoggle == 1) {
        mainradartoggle = 0;
      }
    }

    if (show_menu)
      RenderMenu();
    else
      RenderInfo();

    RenderEsp();

    // Rendering
    ImGui::Render();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    width = display_w;
    height = display_h;
    // glClearColor(clear_color.x * clear_color.w, clear_color.y *
    // clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

std::thread Overlay::Start() {
  std::thread overlay_thr = std::thread(StaticMessageStart, ref(*this));
  return overlay_thr;
}

void Overlay::Clear() {
  running = 0;
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

int Overlay::getWidth() { return width; }

int Overlay::getHeight() { return height; }

void Overlay::DrawLine(ImVec2 a, ImVec2 b, ImColor color, float width) {
  ImGui::GetWindowDrawList()->AddLine(a, b, color, width);
}

void Overlay::DrawBox(ImColor color, float x, float y, float w, float h,
                      float line_w) {
  DrawLine(ImVec2(x, y), ImVec2(x + w, y), color, line_w);
  DrawLine(ImVec2(x, y), ImVec2(x, y + h), color, line_w);
  DrawLine(ImVec2(x + w, y), ImVec2(x + w, y + h), color, line_w);
  DrawLine(ImVec2(x, y + h), ImVec2(x + w, y + h), color, line_w);
}

void Overlay::Text(ImVec2 pos, ImColor color, const char *text_begin,
                   const char *text_end, float wrap_width,
                   const ImVec4 *cpu_fine_clip_rect) {
  ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                                      pos, color, text_begin, text_end,
                                      wrap_width, cpu_fine_clip_rect);
}

void Overlay::String(ImVec2 pos, ImColor color, const char *text) {
  Text(pos, color, text, text + strlen(text), 200, 0);
}

void Overlay::RectFilled(float x0, float y0, float x1, float y1, ImColor color,
                         float rounding, int rounding_corners_flags) {
  ImGui::GetWindowDrawList()->AddRectFilled(
      ImVec2(x0, y0), ImVec2(x1, y1), color, rounding, rounding_corners_flags);
}

void Overlay::ProgressBar(float x, float y, float w, float h, int value,
                          int v_max) {
  ImColor barColor = ImColor(min(510 * (v_max - value) / 100, 255),
                             min(510 * value / 100, 255), 25, 255);

  RectFilled(x, y, x + w, y + ((h / float(v_max)) * (float)value), barColor,
             0.0f, 0);
}

// Seer Hp and Shield bars (never re fixed the armor type so its set to max
// shield)

void DrawQuadFilled(ImVec2 p1, ImVec2 p2, ImVec2 p3, ImVec2 p4, ImColor color) {
  ImGui::GetWindowDrawList()->AddQuadFilled(p1, p2, p3, p4, color);
}
// void DrawHexagon(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const
// ImVec2& p4, const ImVec2& p5, const ImVec2& p6, ImU32 col, float thickness)
// {
// 	ImGui::GetWindowDrawList()->AddHexagon(p1, p2, p3, p4, p5, p6, col,
// thickness);
// }
void DrawHexagonFilled(const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3,
                       const ImVec2 &p4, const ImVec2 &p5, const ImVec2 &p6,
                       ImU32 col) {
  // ImGui::GetWindowDrawList()->AddHexagonFilled(p1, p2, p3, p4, p5, p6, col);
  const ImVec2 points[]{p1, p2, p3, p4, p5, p6};
  ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 6, col);
}

void Overlay::DrawSeerLikeHealth(float x, float y, int shield, int max_shield,
                                 int armorType, int health) {
  // printf("seer(%f,%f), %d/%d, %d, %d\n", x, y, shield, max_shield, armorType,
  // health);

  int bg_offset = 3;
  int bar_width = 158;
  // 4steps...2*3=6
  // 38*4=152 152+6 = 158
  // 5steps...2*4=8
  // 30*5=150 150+8 = 158
  float max_health = 100.0f;
  float shield_step = 25.0f;

  int shield_25 = 30;
  // steps = 5;

  ImVec2 bg1(x - bar_width / 2.0f - bg_offset, y);
  ImVec2 bg2(bg1.x - 10, bg1.y - 16);
  ImVec2 bg3(bg2.x + 5, bg2.y - 7);
  ImVec2 bg4(bg3.x + bar_width + bg_offset, bg3.y);
  ImVec2 bg5(bg4.x + 11, bg4.y + 18);
  ImVec2 bg6(x + bar_width / 2.0f + bg_offset, y);
  DrawHexagonFilled(bg1, bg2, bg3, bg4, bg5, bg6, ImColor(0, 0, 0, 120));

  ImVec2 h1(bg1.x + 3, bg1.y - 4);
  ImVec2 h2(h1.x - 5, h1.y - 8);
  ImVec2 h3(h2.x + (float)health / max_health * bar_width, h2.y);
  ImVec2 h4(h1.x + (float)health / max_health * bar_width, h1.y);
  ImVec2 h3m(h2.x + bar_width, h2.y);
  ImVec2 h4m(h1.x + bar_width, h1.y);
  DrawQuadFilled(h1, h2, h3m, h4m, ImColor(10, 10, 30, 60));
  DrawQuadFilled(h1, h2, h3, h4, WHITE);

  ImColor shieldCracked(97, 97, 97);
  // ImColor shieldCrackedDark(67, 67, 67);

  ImColor shieldCol;
  ImColor shieldColDark;  // not used, but the real seer q has shadow inside
  if (max_shield == 50) { // white
    shieldCol = ImColor(247, 247, 247);
    shieldColDark = ImColor(164, 164, 164);
  } else if (max_shield == 75) { // blue
    shieldCol = ImColor(39, 178, 255);
    shieldColDark = ImColor(27, 120, 210);
  } else if (max_shield == 100) { // purple
    shieldCol = ImColor(206, 59, 255);
    shieldColDark = ImColor(136, 36, 220);
  } else if (max_shield == 100) { // gold
    shieldCol = ImColor(255, 255, 79);
    shieldColDark = ImColor(218, 175, 49);
  } else if (max_shield == 125) { // red
    shieldCol = ImColor(219, 2, 2);
    shieldColDark = ImColor(219, 2, 2);
  } else {
    shieldCol = ImColor(247, 247, 247);
    shieldColDark = ImColor(164, 164, 164);
  }
  int shield_tmp = shield;
  int shield1 = 0;
  int shield2 = 0;
  int shield3 = 0;
  int shield4 = 0;
  int shield5 = 0;
  if (shield_tmp > 25) {
    shield1 = 25;
    shield_tmp -= 25;
    if (shield_tmp > 25) {
      shield2 = 25;
      shield_tmp -= 25;
      if (shield_tmp > 25) {
        shield3 = 25;
        shield_tmp -= 25;
        if (shield_tmp > 25) {
          shield4 = 25;
          shield_tmp -= 25;
          shield5 = shield_tmp;
        } else {
          shield4 = shield_tmp;
        }
      } else {
        shield3 = shield_tmp;
      }
    } else {
      shield2 = shield_tmp;
    }
  } else {
    shield1 = shield_tmp;
  }
  ImVec2 s1(h2.x - 1, h2.y - 2);
  ImVec2 s2(s1.x - 3, s1.y - 5);
  ImVec2 s3(s2.x + shield1 / shield_step * shield_25, s2.y);
  ImVec2 s4(s1.x + shield1 / shield_step * shield_25, s1.y);
  ImVec2 s3m(s2.x + shield_25, s2.y);
  ImVec2 s4m(s1.x + shield_25, s1.y);

  ImVec2 ss1(s4m.x + 2, s1.y);
  ImVec2 ss2(s3m.x + 2, s2.y);
  ImVec2 ss3(ss2.x + shield2 / shield_step * shield_25, s2.y);
  ImVec2 ss4(ss1.x + shield2 / shield_step * shield_25, s1.y);
  ImVec2 ss3m(ss2.x + shield_25, s2.y);
  ImVec2 ss4m(ss1.x + shield_25, s1.y);

  ImVec2 sss1(ss4m.x + 2, s1.y);
  ImVec2 sss2(ss3m.x + 2, s2.y);
  ImVec2 sss3(sss2.x + shield3 / shield_step * shield_25, s2.y);
  ImVec2 sss4(sss1.x + shield3 / shield_step * shield_25, s1.y);
  ImVec2 sss3m(sss2.x + shield_25, s2.y);
  ImVec2 sss4m(sss1.x + shield_25, s1.y);

  ImVec2 ssss1(sss4m.x + 2, s1.y);
  ImVec2 ssss2(sss3m.x + 2, s2.y);
  ImVec2 ssss3(ssss2.x + shield4 / shield_step * shield_25, s2.y);
  ImVec2 ssss4(ssss1.x + shield4 / shield_step * shield_25, s1.y);
  ImVec2 ssss3m(ssss2.x + shield_25, s2.y);
  ImVec2 ssss4m(ssss1.x + shield_25, s1.y);

  ImVec2 sssss1(ssss4m.x + 2, s1.y);
  ImVec2 sssss2(ssss3m.x + 2, s2.y);
  ImVec2 sssss3(sssss2.x + shield5 / shield_step * shield_25, s2.y);
  ImVec2 sssss4(sssss1.x + shield5 / shield_step * shield_25, s1.y);
  ImVec2 sssss3m(sssss2.x + shield_25, s2.y);
  ImVec2 sssss4m(sssss1.x + shield_25, s1.y);
  if (max_shield == 50) {
    if (shield <= 25) {
      if (shield < 25) {
        DrawQuadFilled(s1, s2, s3m, s4m, shieldCracked);
        DrawQuadFilled(ss1, ss2, ss3m, ss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(s1, s2, s3, s4, shieldCol);

    } else if (shield <= 50) {
      DrawQuadFilled(s1, s2, s3, s4, shieldCol);
      if (shield != 50) {
        DrawQuadFilled(ss1, ss2, ss3m, ss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(ss1, ss2, ss3, ss4, shieldCol);
    }
  } else if (max_shield == 75) {
    if (shield <= 25) {
      if (shield < 25) {
        DrawQuadFilled(s1, s2, s3m, s4m, shieldCracked);
        DrawQuadFilled(ss1, ss2, ss3m, ss4m, shieldCracked);
        DrawQuadFilled(sss1, sss2, sss3m, sss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(s1, s2, s3, s4, shieldCol);

    } else if (shield <= 50) {
      DrawQuadFilled(s1, s2, s3, s4, shieldCol);
      if (shield < 50) {
        DrawQuadFilled(ss1, ss2, ss3m, ss4m, shieldCracked);
        DrawQuadFilled(sss1, sss2, sss3m, sss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(ss1, ss2, ss3, ss4, shieldCol);
    } else if (shield <= 75) {
      DrawQuadFilled(s1, s2, s3, s4, shieldCol);
      DrawQuadFilled(ss1, ss2, ss3, ss4, shieldCol);
      if (shield < 75) {
        DrawQuadFilled(sss1, sss2, sss3m, sss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(sss1, sss2, sss3, sss4, shieldCol);
    }
  } else if (max_shield == 100) {
    if (shield <= 25) {
      if (shield < 25) {
        DrawQuadFilled(s1, s2, s3m, s4m, shieldCracked);
        DrawQuadFilled(ss1, ss2, ss3m, ss4m, shieldCracked);
        DrawQuadFilled(sss1, sss2, sss3m, sss4m, shieldCracked);
        DrawQuadFilled(ssss1, ssss2, ssss3m, ssss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(s1, s2, s3, s4, shieldCol);

    } else if (shield <= 50) {
      DrawQuadFilled(s1, s2, s3, s4, shieldCol);
      if (shield < 50) {
        DrawQuadFilled(ss1, ss2, ss3m, ss4m, shieldCracked);
        DrawQuadFilled(sss1, sss2, sss3m, sss4m, shieldCracked);
        DrawQuadFilled(ssss1, ssss2, ssss3m, ssss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(ss1, ss2, ss3, ss4, shieldCol);
    } else if (shield <= 75) {
      DrawQuadFilled(s1, s2, s3, s4, shieldCol);
      DrawQuadFilled(ss1, ss2, ss3, ss4, shieldCol);
      if (shield < 75) {
        DrawQuadFilled(sss1, sss2, sss3m, sss4m, shieldCracked);
        DrawQuadFilled(ssss1, ssss2, ssss3m, ssss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(sss1, sss2, sss3, sss4, shieldCol);
    } else if (shield <= 100) {
      DrawQuadFilled(s1, s2, s3, s4, shieldCol);
      DrawQuadFilled(ss1, ss2, ss3, ss4, shieldCol);
      DrawQuadFilled(sss1, sss2, sss3, sss4, shieldCol);
      if (shield < 100) {
        DrawQuadFilled(ssss1, ssss2, ssss3m, ssss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(ssss1, ssss2, ssss3, ssss4, shieldCol);
    }
  } else if (max_shield == 125) {
    if (shield <= 25) {
      if (shield < 25) {
        DrawQuadFilled(s1, s2, s3m, s4m, shieldCracked);
        DrawQuadFilled(ss1, ss2, ss3m, ss4m, shieldCracked);
        DrawQuadFilled(sss1, sss2, sss3m, sss4m, shieldCracked);
        DrawQuadFilled(ssss1, ssss2, ssss3m, ssss4m, shieldCracked);
        DrawQuadFilled(sssss1, sssss2, sssss3m, sssss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(s1, s2, s3, s4, shieldCol);

    } else if (shield <= 50) {
      DrawQuadFilled(s1, s2, s3, s4, shieldCol);
      if (shield < 50) {
        DrawQuadFilled(ss1, ss2, ss3m, ss4m, shieldCracked);
        DrawQuadFilled(sss1, sss2, sss3m, sss4m, shieldCracked);
        DrawQuadFilled(ssss1, ssss2, ssss3m, ssss4m, shieldCracked);
        DrawQuadFilled(sssss1, sssss2, sssss3m, sssss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(ss1, ss2, ss3, ss4, shieldCol);
    } else if (shield <= 75) {
      DrawQuadFilled(s1, s2, s3, s4, shieldCol);
      DrawQuadFilled(ss1, ss2, ss3, ss4, shieldCol);
      if (shield < 75) {
        DrawQuadFilled(sss1, sss2, sss3m, sss4m, shieldCracked);
        DrawQuadFilled(ssss1, ssss2, ssss3m, ssss4m, shieldCracked);
        DrawQuadFilled(sssss1, sssss2, sssss3m, sssss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(sss1, sss2, sss3, sss4, shieldCol);
    } else if (shield <= 100) {
      DrawQuadFilled(s1, s2, s3, s4, shieldCol);
      DrawQuadFilled(ss1, ss2, ss3, ss4, shieldCol);
      DrawQuadFilled(sss1, sss2, sss3, sss4, shieldCol);
      if (shield < 100) {
        DrawQuadFilled(ssss1, ssss2, ssss3m, ssss4m, shieldCracked);
        DrawQuadFilled(sssss1, sssss2, sssss3m, sssss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(ssss1, ssss2, ssss3, ssss4, shieldCol);
    } else if (shield <= 125) {
      DrawQuadFilled(s1, s2, s3, s4, shieldCol);
      DrawQuadFilled(ss1, ss2, ss3, ss4, shieldCol);
      DrawQuadFilled(sss1, sss2, sss3, sss4, shieldCol);
      DrawQuadFilled(ssss1, ssss2, ssss3, ssss4, shieldCol);
      if (shield < 125) {
        DrawQuadFilled(sssss1, sssss2, sssss3m, sssss4m, shieldCracked);
      }
      if (shield != 0)
        DrawQuadFilled(sssss1, sssss2, sssss3, sssss4, shieldCol);
    }
  }
}
