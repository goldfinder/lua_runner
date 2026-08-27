#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h> // For usleep

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// Global delay in microseconds (0 = unlocked / no sleep)
static useconds_t frame_delay_us = 16666; // Defaults to ~60 FPS

// Recalculates microsecond delay from FPS target
static void update_frame_delay(int mframes) {
    if (mframes <= 0) {
        frame_delay_us = 0; // Toggle usleep off
    } else {
        frame_delay_us = (useconds_t)(1000000 / mframes);
    }
}

// Exposed Lua function: set_mframes(fps_number)
static int api_set_mframes(lua_State *L) {
    int mframes = (int)luaL_checkinteger(L, 1);
    update_frame_delay(mframes);
    return 0;
}

// Custom log function
static int api_log(lua_State *L) {
    const char *msg = luaL_checkstring(L, 1);
    printf("[ENGINE]: %s\n", msg);
    return 0;
}

// Bind API functions into Lua
void register_lua_api(lua_State *L) {
    lua_register(L, "log", api_log);
    lua_register(L, "set_mframes", api_set_mframes);
}

int main(int argc, char *argv[]) {
    const char *target_script = "Main.lua";

    if (access(target_script, F_OK) != 0) {
        fprintf(stderr, "[FATAL]: Required file '%s' not found.\n", target_script);
        return 1;
    }

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    register_lua_api(L);

    if (luaL_dofile(L, target_script) != LUA_OK) {
        fprintf(stderr, "[LUA ERROR]: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    // Call on_start hook
    lua_getglobal(L, "on_start");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "[RUNTIME ERROR]: %s\n", lua_tostring(L, -1));
        }
    } else {
        lua_pop(L, 1);
    }

    // Main Loop
    bool running = true;
    while (running) {
        lua_getglobal(L, "on_update");
        if (lua_isfunction(L, -1)) {
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                if (lua_isboolean(L, -1) && !lua_toboolean(L, -1)) {
                    running = false;
                }
            } else {
                fprintf(stderr, "[RUNTIME ERROR]: %s\n", lua_tostring(L, -1));
                running = false;
            }
            lua_pop(L, 1);
        } else {
            lua_pop(L, 1);
        }

        // Toggleable usleep: only sleep if a delay is configured
        if (frame_delay_us > 0) {
            usleep(frame_delay_us);
        }
    }

    lua_close(L);
    printf("[ENGINE]: Shutdown cleanly.\n");
    return 0;
}
