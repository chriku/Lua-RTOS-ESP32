/*
 * Copyright (C) 2015 - 2018, Thomas E. Horner (whitecatboard.org@horner.it)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *     * The WHITECAT logotype cannot be changed, you can remove it, but you
 *       cannot change it in any way. The WHITECAT logotype is:
 *
 *          /\       /\
 *         /  \_____/  \
 *        /_____________\
 *        W H I T E C A T
 *
 *     * Redistributions in binary form must retain all copyright notices printed
 *       to any local or remote output device. This include any reference to
 *       Lua RTOS, whitecatboard.org, Lua, and other copyright notices that may
 *       appear in the future.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Lua RTOS, Lua http net module
 *
 */

#include "sdkconfig.h"

#if CONFIG_LUA_RTOS_USE_HTTP_SERVER

#include "net.h"

#include "lwip/err.h"

#include <drivers/net.h>

static int lhttp_start(lua_State* L) {
	lua_getfield(L,LUA_REGISTRYINDEX,"my_http_start");
	lua_call(L,0,0);
	return 0;
}

static int lhttp_stop(lua_State* L) {
	lua_getfield(L,LUA_REGISTRYINDEX,"my_http_stop");
	lua_call(L,0,0);
	return 0;
}

static int lhttp_print(lua_State* L) {
	int top=lua_gettop(L);
	lua_getfield(L,LUA_REGISTRYINDEX,"my_http_print");
	lua_rotate(L,1,1);
	lua_call(L,top,0);
	return 0;
}

static int lhttp_status(lua_State* L) {
	int top=lua_gettop(L);
	lua_getfield(L,LUA_REGISTRYINDEX,"my_http_status");
	lua_rotate(L,1,1);
	lua_call(L,top,0);
	return 1;
}

static const LUA_REG_TYPE http_map[] = {
    { LSTRKEY( "start" ),	 LFUNCVAL( lhttp_start   ) },
    { LSTRKEY( "stop"  ),	 LFUNCVAL( lhttp_stop    ) },
    { LSTRKEY( "print_chunk"  ),	 LFUNCVAL( lhttp_print    ) },
    { LSTRKEY( "set_headers"  ),	 LFUNCVAL( lhttp_status    ) },
	{ LNILKEY, LNILVAL }
};

#endif
