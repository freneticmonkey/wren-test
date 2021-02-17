#include <wren.h>

#include "wren_util.h"

#include <stdio.h>
#include <string.h>

static WrenVM *vm = NULL;

//This is a simple test runner that serves one purpose:
//To run the language level tests and benchmarks for Wren.
//It is not a general purpose vm or REPL.
//See wren-cli if you're looking for that.

static WrenVM *initVM()
{
    WrenConfiguration config;
    wrenInitConfiguration(&config);

    config.resolveModuleFn = resolveModule;
    config.loadModuleFn = readModule;
    config.writeFn = vm_write;
    config.errorFn = reportError;

    // if(isAPITest) {
    //   config.bindForeignClassFn = APITest_bindForeignClass;
    //   config.bindForeignMethodFn = APITest_bindForeignMethod;
    // }

    // Since we're running in a standalone process, be generous with memory.
    config.initialHeapSize = 1024 * 1024 * 100;
    return wrenNewVM(&config);
}

void handle_result(WrenInterpretResult result)
{
    if (result == WREN_RESULT_COMPILE_ERROR)
    {
        fprintf(stderr, "Error calling Wren: Compile Error");
        exit(WREN_RESULT_COMPILE_ERROR);
    }
    if (result == WREN_RESULT_RUNTIME_ERROR)
    {
        fprintf(stderr, "Error calling Wren: Runtime Error");
        exit(WREN_RESULT_RUNTIME_ERROR);
    }
}

int main(int argc, const char *argv[])
{

    int handled = handle_args(argc, argv);
    if (handled != 0)
        return handled;

    int exitCode = 0;
    const char *file = argv[1];

    vm = initVM();
    WrenInterpretResult result = runFile(vm, file);

    handle_result(result);

    // Load the class into slot 0.
    wrenEnsureSlots(vm, 1);
    wrenGetVariable(vm, "./script", "Script", 0);

    // Call the script constructor
    WrenHandle *constructor = wrenMakeCallHandle(vm, "Script.new");
    WrenHandle *on_update = wrenMakeCallHandle(vm, "Script.on_update");

    result = wrenCall(vm, constructor);

    handle_result(result);

    WrenHandle *scriptClassInstance = wrenGetSlotHandle(vm, 0);

    // Set Script instance
    wrenSetSlotHandle(vm, 0, scriptClassInstance);
    // Set Elapsed Time
    wrenSetSlotDouble(vm, 1, 0.5f);

    // Call Script.on_update
    result = wrenCall(vm, on_update);

    handle_result(result);

    // Clean up
    wrenReleaseHandle(vm, scriptClassInstance);
    wrenReleaseHandle(vm, constructor);

    wrenFreeVM(vm);

    return exitCode;
}