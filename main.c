#include <wren.h>
#include <wren_value.h>
#include <wren_vm.h>
#include <wren_debug.h>

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


// Test Class instantiation and calling a function on the instantiated instance
// Expected result:
//
//      New Script!
//      On Update
//      Elapsed Time: 0.5
//      Move Speed: 1
//      New Move Speed: 1.5
//      Program ended with exit code: 0
//
void test_class_access()
{   
    WrenInterpretResult result;

    // Load the class into slot 0.
    wrenEnsureSlots(vm, 1);
    
    if ( !wrenHasVariable(vm, "../script", "Script"))
    {
        fprintf(stderr, "Script class is missing");
        exit(-1);
    }
    
    // Get a handle to the Script class
    wrenGetVariable(vm, "../script", "Script", 0);
    WrenHandle *script_class = wrenGetSlotHandle(vm, 0);
    
    // Set the Script object as the receiver
    //wrenSetSlotHandle(vm, 0, script_class);
    
    // Get the handles to the script constructor and update methods
    WrenHandle *constructor = wrenMakeCallHandle(vm, "new()");
    WrenHandle *on_update = wrenMakeCallHandle(vm, "on_update(_)");

    // Create a script instance
    wrenSetSlotHandle(vm, 0, script_class);
    result = wrenCall(vm, constructor);

    // Check the result
    handle_result(result);

    // Get a handle to the new instance
    WrenHandle *script_instance = wrenGetSlotHandle(vm, 0);

    // Set Script instance
    wrenSetSlotHandle(vm, 0, script_instance);

    // Set Elapsed Time parameter
    wrenSetSlotDouble(vm, 1, 0.5f);

    // Call Script.on_update
    result = wrenCall(vm, on_update);

    // Check the result
    handle_result(result);

    // Clean up
    wrenReleaseHandle(vm, script_instance);
    wrenReleaseHandle(vm, constructor);
    wrenReleaseHandle(vm, script_class);
}

bool starts_with(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

size_t strchrpos(const char *str, int c)
{
    char *p = strchr(str, c);
    if (p != NULL)
    {
        return p - str + 1;
    }
    return -1;
}

bool match_method(const char *method1, const char *method2)
{
    size_t len_method1 = strchrpos(method1, '('),
           len_method2 = strchrpos(method2, '(');
    
    return len_method1 < len_method2 ? false : memcmp(method1, method2, len_method1) == 0;
}


// Reflection method
// Processes the Script class into array using the following logic
//
// constructors
//  prefixed by 'init'
// accessors (getters/setters)
//  named pairs with single parameters
// functions
//  everything else
//
// Expected result
//
//        Script
//
//        Constructors
//        --------------
//        init new(_)
//        init new()
//
//        Accessors
//        --------------
//        enabled
//        zomg
//        move_speed
//
//        Functions
//        --------------
//        on_update(_)
//
//
void test_class_reflection()
{
    int METHOD_MAX = 64;
    int METHOD_MAX_LENGTH = 128;

    int  method_count = 0;
    char methods[METHOD_MAX][METHOD_MAX_LENGTH];
    
    int  constructor_count = 0;
    char constructors[8][METHOD_MAX_LENGTH];
    
    int  accessor_count = 0;
    char accessors[32][METHOD_MAX_LENGTH];
    
    int  function_count = 0;
    char functions[16][METHOD_MAX_LENGTH];
    
    // Load the class into slot 0.
    wrenEnsureSlots(vm, 1);
    
    if ( !wrenHasVariable(vm, "../script", "Script"))
    {
        fprintf(stderr, "Script class is missing");
        exit(-1);
    }
    
    // Get a handle to the Script class
    wrenGetVariable(vm, "../script", "Script", 0);
    WrenHandle *script_class = wrenGetSlotHandle(vm, 0);

    // If the script class variable is valid
    if ( IS_CLASS(script_class->value) )
    {
        // Print the class name
        ObjClass* cls = AS_CLASS(script_class->value);
        printf("%.*s\n", cls->name->length, cls->name->value);

        wrenDumpValue(script_class->value);
        
        // For each of the class' methods
        for(int i = 0; i < cls->methods.count; ++i) 
        {
            // if the method is one that we can access
            Method *method = &(cls->methods.data[i]);
            if(method->type != METHOD_PRIMITIVE && method->type != METHOD_NONE) 
            {
                // Store the method name
                char * name = method->as.closure->fn->debug->name;
                size_t length = strlen(name);
                
                // truncate if necessary
                if (length > METHOD_MAX_LENGTH)
                    length = METHOD_MAX_LENGTH;
                
                strncpy(methods[method_count], name, length);
//                printf("%s\n", method->as.closure->fn->debug->name);
                method_count++;
                if (method_count >= METHOD_MAX)
                {
                    printf("Maximum functions reached\n");
                    break;
                }
            }
        }
    }
    
    // filter the methods into the correct categories
    for (int i = 0; i < method_count; i++)
    {
        // If this function has already been handled
        if (methods[i][0] == '\0')
            continue;
        
        // if a constructor
        else if ( starts_with("init", methods[i]) )
        {
            size_t length = strchrpos(methods[i], ')');
            strncpy(constructors[constructor_count], methods[i], length);
            constructors[constructor_count][length] = '\0';
            constructor_count++;
            
            methods[i][0] = '\0';
        }
        else
        {
            // check if matching pair exists in methods list - an accessor
            bool is_accessor = false;
            for (int j=0; j < method_count; j++)
            {
                if ( (i == j) || methods[j][0] == '\0')
                    continue;
                
                if (match_method(methods[i], methods[j]))
                {
                    // store accessor
                    size_t length = strchrpos(methods[i], '(')-1;
                    strncpy(accessors[accessor_count], methods[i], length);
                    accessors[accessor_count][length] = '\0';
                    accessor_count++;
                    
                    // mark both methods as processed
                    methods[i][0] = '\0';
                    methods[j][0] = '\0';
                    is_accessor = true;
                    
                    break;
                }
            }
            
            if (!is_accessor)
            {
                size_t length = strchrpos(methods[i], ')');
                strncpy(functions[function_count], methods[i], length);
                functions[function_count][length] = '\0';
                function_count++;
                
                methods[i][0] = '\0';
            }
        }
    }
    
    printf("\nConstructors\n--------------\n");
    for (int i = 0; i < constructor_count; i++)
        printf("%s\n", constructors[i]);
    
    printf("\nAccessors\n--------------\n");
    for (int i = 0; i < accessor_count; i++)
        printf("%s\n", accessors[i]);
    
    printf("\nFunctions\n--------------\n");
    for (int i = 0; i < function_count; i++)
        printf("%s\n", functions[i]);
        
    wrenReleaseHandle(vm, script_class);
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

    // Process class manipulation
    test_class_access();
    
    // Process class method reflection
    test_class_reflection();

    wrenFreeVM(vm);

    return exitCode;
}


