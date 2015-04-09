/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, FREE of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "core/os_def.h"
#include "util/cjson.h"

// Parse text to JSON, then render back to text, and print!
static void
_json_do_text(const char *text) {
    char* out;
    cJSON* json = cJSON_Parse(text);
    if (!json) {
        fprintf(stderr, "Error before: [%s]\n", cJSON_GetErrorPtr());
    } else {
        out = cJSON_Print(json);
        cJSON_Delete(json);
        // printf("%s\n", out);
        FREE(out);
    }
}

// Read a file, parse, render back, etc.
static void
_json_do_file(const char* param) {
    const char* filename = param ? param : "./json_test_file";
    FILE *f = fopen(filename,"rb");
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = (char*)MALLOC(len + 1);
    fread(data, 1, len, f);
    fclose(f);
    _json_do_text(data);
    FREE(data);
}

// Used by some code below as an example datatype.
struct JSON_RECORD {
    const char* precision;
    double lat, lon;
    const char *address, *city, *state, *zip, *country;
};

// Create a bunch of objects as demonstration.
static void
_json_create() {
    cJSON *root, *fmt, *img, *thm, *fld;
    char *out;
    int i;
    const char *strings[7] = {
        "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
    };
    int numbers[3][3] = {
        {0,-1,0},
        {1,0,0},
        {0,0,1}
    };
    struct JSON_RECORD fields[2] = {
        {"zip",37.7668,-1.223959e+2,"","SAN FRANCISCO","CA","94107","US"},
        {"zip",37.371991,-1.22026e+2,"","SUNNYVALE","CA","94085","US"}
    };
    int ids[4] = {
        116, 943, 234, 38793
    };

    // Our "Video" datatype:
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
    cJSON_AddItemToObject(root, "format", fmt=cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "type", "rect");
    cJSON_AddNumberToObject(fmt, "width", 1920);
    cJSON_AddNumberToObject(fmt, "height", 1080);
    cJSON_AddFalseToObject (fmt, "interlace");
    cJSON_AddNumberToObject(fmt, "frame rate", 24);

    out = cJSON_Print(root);
    cJSON_Delete(root);
    printf("%s\n", out);
    FREE(out);
    // Print to text, Delete the cJSON, print it, release the string.
    root = cJSON_CreateStringArray(strings, 7);

    out = cJSON_Print(root);
    cJSON_Delete(root);
    printf("%s\n", out);
    FREE(out);

    root = cJSON_CreateArray();
    for (i=0; i<3; i++) {
        cJSON_AddItemToArray(root, cJSON_CreateIntArray(numbers[i], 3));
    }

    out = cJSON_Print(root);
    cJSON_Delete(root);
    printf("%s\n", out);
    FREE(out);

    // Our "gallery" item:
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "Image", img=cJSON_CreateObject());
    cJSON_AddNumberToObject(img, "Width", 800);
    cJSON_AddNumberToObject(img, "Height", 600);
    cJSON_AddStringToObject(img, "Title", "View from 15th Floor");
    cJSON_AddItemToObject(img, "Thumbnail", thm=cJSON_CreateObject());
    cJSON_AddStringToObject(thm, "Url", "http://www.example.com/image/481989943");
    cJSON_AddNumberToObject(thm, "Height", 125);
    cJSON_AddStringToObject(thm, "Width", "100");
    cJSON_AddItemToObject(img, "IDs", cJSON_CreateIntArray(ids,4));

    out = cJSON_Print(root);
    cJSON_Delete(root);
    printf("%s\n", out);
    FREE(out);

    root = cJSON_CreateArray();
    for (i=0; i<2; i++) {
        cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
        cJSON_AddStringToObject(fld, "precision", fields[i].precision);
        cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
        cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
        cJSON_AddStringToObject(fld, "Address", fields[i].address);
        cJSON_AddStringToObject(fld, "City", fields[i].city);
        cJSON_AddStringToObject(fld, "State", fields[i].state);
        cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
        cJSON_AddStringToObject(fld, "Country", fields[i].country);
    }

    out = cJSON_Print(root);
    cJSON_Delete(root);
    printf("%s\n", out);
    FREE(out);
}

int
test_util_cjson_text(const char* param) {
    char text[] = "[\n"
        "     {\n"
        "     \"precision\": \"zip\",\n"
        "     \"Latitude\":  37.7668,\n"
        "     \"Longitude\": -122.3959,\n"
        "     \"Address\":   \"\",\n"
        "     \"City\":      \"SAN FRANCISCO\",\n"
        "     \"State\":     \"CA\",\n"
        "     \"Zip\":       \"94107\",\n"
        "     \"Country\":   \"US\"\n"
        "     },\n"
        "     {\n"
        "     \"precision\": \"zip\",\n"
        "     \"Latitude\":  37.371991,\n"
        "     \"Longitude\": -122.026020,\n"
        "     \"Address\":   \"\",\n"
        "     \"City\":      \"SUNNYVALE\",\n"
        "     \"State\":     \"CA\",\n"
        "     \"Zip\":       \"94085\",\n"
        "     \"Country\":   \"US\"\n"
        "     }\n"
        "]";
    _json_do_text(text);
    return 0;
}

int
test_util_cjson_file(const char* param) {
    _json_do_file(param);
   return 0; 
}

int
test_util_cjson_create(const char* param) {
    _json_create();
    return 0;
}

