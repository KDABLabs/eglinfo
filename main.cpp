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

struct attrib_t {
    EGLint attribute;
    const char* displayName;
    enum_t* enumMap;
    int enumMapSize;
};

#define A_NUM(x) { x, #x, 0, 0 }
#define A_MAP(x, map) { x, #x, map, sizeof(map) / sizeof(enum_t) }

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
    A_NUM(EGL_RENDERABLE_TYPE),
    A_NUM(EGL_SAMPLE_BUFFERS),
    A_NUM(EGL_SAMPLES),
    A_NUM(EGL_STENCIL_SIZE),
    A_NUM(EGL_SURFACE_TYPE),
    A_MAP(EGL_TRANSPARENT_TYPE, transparentTypeMap),
    A_NUM(EGL_TRANSPARENT_RED_VALUE),
    A_NUM(EGL_TRANSPARENT_GREEN_VALUE),
    A_NUM(EGL_TRANSPARENT_BLUE_VALUE)
};

#undef A

static const int attributesSize = sizeof(attributes) / sizeof(attrib_t);


int main(int argc, char** argv)
{
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
            EGLint value;
            EGLBoolean result = eglGetConfigAttrib(display, configs[i], attributes[j].attribute, &value);
            cout << "  " << attributes[j].displayName << ": ";
            if (result) {
                if (attributes[j].enumMap) {
                    for (int k = 0; k < attributes[j].enumMapSize; ++k) {
                        if (value == attributes[j].enumMap[k].value)
                            cout << attributes[j].enumMap[k].displayName;
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

