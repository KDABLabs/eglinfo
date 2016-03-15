/*
    Copyright (C) 2012 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Author: Volker Krause <volker.krause@kdab.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <cstdlib>
#include <iostream>

using namespace std;

struct enum_t {
    EGLint value;
    const char* displayName;
};

static enum_t boolMap[] {
    { EGL_TRUE, "true" },
    { EGL_FALSE, "false" }
};

static enum_t bufferTypeMap[] {
    { EGL_RGB_BUFFER, "RGB" },
    { EGL_LUMINANCE_BUFFER, "Luminance" }
};

static enum_t caveatMap[] {
    { EGL_NONE, "none" },
    { EGL_SLOW_CONFIG, "slow" },
    { EGL_NON_CONFORMANT_CONFIG, "non-conformant" }
};

static enum_t transparentTypeMap[] {
    { EGL_NONE, "none" },
    { EGL_TRANSPARENT_RGB, "transparent RGB" }
};

static enum_t surfaceTypeMap[] {
    { EGL_PBUFFER_BIT, "pbuffer" },
    { EGL_PIXMAP_BIT, "pixmap" },
    { EGL_WINDOW_BIT, "window" },
    { EGL_VG_COLORSPACE_LINEAR_BIT, "VG (linear colorspace)" },
    { EGL_VG_ALPHA_FORMAT_PRE_BIT, "VG (alpha format pre)" },
    { EGL_MULTISAMPLE_RESOLVE_BOX_BIT, "multisample resolve box" },
    { EGL_SWAP_BEHAVIOR_PRESERVED_BIT, "swap behavior preserved" }
};

static enum_t renderableTypeMap[] {
    { EGL_OPENGL_ES_BIT, "OpenGL ES" },
    { EGL_OPENVG_BIT, "OpenVG" },
    { EGL_OPENGL_ES2_BIT, "OpenGL ES2" },
    { EGL_OPENGL_BIT, "OpenGL" }
};


struct attrib_t {
    EGLint attribute;
    const char* displayName;
    enum_t* enumMap;
    int enumMapSize;
    bool isFlag;
};

#define A_NUM(x) { x, #x, 0, 0, false }
#define A_MAP(x, map) { x, #x, map, sizeof(map) / sizeof(enum_t), false }
#define A_FLAG(x, map) { x, #x, map, sizeof(map) / sizeof(enum_t), true }

static attrib_t attributes[] {
    A_NUM(EGL_ALPHA_SIZE),
    A_NUM(EGL_ALPHA_MASK_SIZE),
    A_MAP(EGL_BIND_TO_TEXTURE_RGB, boolMap),
    A_MAP(EGL_BIND_TO_TEXTURE_RGBA, boolMap),
    A_NUM(EGL_BLUE_SIZE),
    A_NUM(EGL_BUFFER_SIZE),
    A_MAP(EGL_COLOR_BUFFER_TYPE, bufferTypeMap),
    A_MAP(EGL_CONFIG_CAVEAT, caveatMap),
    A_NUM(EGL_CONFIG_ID),
    A_NUM(EGL_CONFORMANT),
    A_NUM(EGL_DEPTH_SIZE),
    A_NUM(EGL_GREEN_SIZE),
    A_NUM(EGL_LEVEL),
    A_NUM(EGL_LUMINANCE_SIZE),
    A_NUM(EGL_MAX_PBUFFER_WIDTH),
    A_NUM(EGL_MAX_PBUFFER_HEIGHT),
    A_NUM(EGL_MAX_PBUFFER_PIXELS),
    A_NUM(EGL_MAX_SWAP_INTERVAL),
    A_NUM(EGL_MIN_SWAP_INTERVAL),
    A_MAP(EGL_NATIVE_RENDERABLE, boolMap),
    A_NUM(EGL_NATIVE_VISUAL_ID),
    A_NUM(EGL_NATIVE_VISUAL_TYPE),
    A_NUM(EGL_RED_SIZE),
    A_FLAG(EGL_RENDERABLE_TYPE, renderableTypeMap),
    A_NUM(EGL_SAMPLE_BUFFERS),
    A_NUM(EGL_SAMPLES),
    A_NUM(EGL_STENCIL_SIZE),
    A_FLAG(EGL_SURFACE_TYPE, surfaceTypeMap),
    A_MAP(EGL_TRANSPARENT_TYPE, transparentTypeMap),
    A_NUM(EGL_TRANSPARENT_RED_VALUE),
    A_NUM(EGL_TRANSPARENT_GREEN_VALUE),
    A_NUM(EGL_TRANSPARENT_BLUE_VALUE)
};

#undef A_NUM
#undef A_MAP
#undef A_FLAG

static const int attributesSize = sizeof(attributes) / sizeof(attrib_t);


int main(int argc, char** argv)
{
    const char* clientExts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    cout << "Client extensions: " << clientExts << endl << endl;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        cerr << "Could not obtain EGL display!" << endl;
        exit(1);
    }

    EGLint majorVersion, minorVersion;
    if (!eglInitialize(display, &majorVersion, &minorVersion)) {
        cerr << "Could not initialize EGL!" << endl;
        exit(1);
    }

    cout << "EGL version: " << majorVersion << "." << minorVersion << endl;
    const char* clientAPIs = eglQueryString(display, EGL_CLIENT_APIS);
    cout << "Client APIs for display: " << clientAPIs << endl;
    const char* vendor = eglQueryString(display, EGL_VENDOR);
    cout << "Vendor: " << vendor << endl;
    const char* displayExts = eglQueryString(display, EGL_EXTENSIONS);
    cout << "Display extensions: " << displayExts << endl;

    EGLint numConfigs;
    if (!eglGetConfigs(display, 0, 0, &numConfigs) && numConfigs > 0) {
        cerr << "Could not retrieve the number of EGL configurations!" << endl;
        exit(1);
    }

    cout << "Found " << numConfigs << " configurations." << endl << endl;

    EGLConfig *configs = new EGLConfig[numConfigs];
    if (!eglGetConfigs(display, configs, numConfigs, &numConfigs)) {
        cerr << "Could not retrieve EGL configurations!" << endl;
        exit(1);
    }

    for (int i = 0; i < numConfigs; ++i) {
        cout << "Configuration " << i << ":" << endl;
        for (int j = 0; j < attributesSize; ++j) {
            attrib_t *attr = &attributes[j];
            EGLint value;
            EGLBoolean result = eglGetConfigAttrib(display, configs[i], attr->attribute, &value);
            cout << "  " << attr->displayName << ": ";
            if (result) {
                if (attr->enumMap) {
                    bool firstEntry = true;
                    for (int k = 0; k < attr->enumMapSize; ++k) {
                        enum_t *enumValue = &attr->enumMap[k];
                        if (value == enumValue->value && !attr->isFlag) {
                            cout <<  enumValue->displayName;
                        } else if (value & enumValue->value && attr->isFlag) {
                            if (!firstEntry)
                                cout << ", ";
                            cout << enumValue->displayName;
                            firstEntry = false;
                        }
                    }
                } else {
                    cout << value;
                }
            } else {
                cout << "<failed>";
            }
            cout << endl;
        }
        cout << endl;
    }

    delete[] configs;
}
