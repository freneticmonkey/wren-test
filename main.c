#include <wren.h>
#include <wren_value.h>
#include <wren_vm.h>
#include <wren_debug.h>

#include <stdio.h>
#include <string.h>

#include "wren_util.h"
#include "vec3.h"

#define METHOD_MAX 64
#define METHOD_MAX_LENGTH 128

static WrenVM *vm = NULL;

WrenForeignClassMethods bindForeignClass(WrenVM* vm, 
                                         const char* module, 
                                         const char* className);

WrenForeignMethodFn bindForeignMethod(WrenVM* vm,
                                      const char* moduleName,
                                      const char* className,
                                      bool isStatic,
                                      const char* signature);

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

    config.bindForeignClassFn = bindForeignClass;
    config.bindForeignMethodFn = bindForeignMethod;

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
    
    if ( !wrenHasVariable(vm, "script", "Script"))
    {
        fprintf(stderr, "Script class is missing");
        exit(-1);
    }
    
    // Get a handle to the Script class
    wrenGetVariable(vm, "script", "Script", 0);
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

void type_to_string(WrenType type)
{
    switch(type)
    {
    case WREN_TYPE_BOOL:
        printf("BOOL\n");
        break;
    case WREN_TYPE_NUM:
        printf("NUM\n");
        break;
    case WREN_TYPE_FOREIGN:
        printf("FOREIGN\n");
        break;
    case WREN_TYPE_LIST:
        printf("LIST\n");
        break;
    case WREN_TYPE_MAP:
        printf("MAP\n");
        break;
    case WREN_TYPE_NULL:
        printf("NULL\n");
        break;
    case WREN_TYPE_STRING:
        printf("STRING\n");
        break;

        // The object is of a type that isn't accessible by the C API.
    default:
        printf("UNKNOWN\n");
    }
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
//      New Script!
//      On Update
//      Elapsed Time: 0.5
//      Move Speed: 1
//      New Move Speed: 1.5
//      Script
//      [class Script 00F12170]
//      New Script!
//      
//      Constructors
//      --------------
//      init new(_)
//      init new()
//      
//      Accessors
//      --------------
//      enabled: Type: BOOL
//      zomg: Type: STRING
//      move_speed: Type: NUM
//      
//      Functions
//      --------------
//      on_update(_)
//
//
void test_class_reflection()
{
    WrenInterpretResult result;

    int  method_count = 0;
    char methods[METHOD_MAX][METHOD_MAX_LENGTH];
    
    int  constructor_count = 0;
    char constructors[8][METHOD_MAX_LENGTH];
    
    int  accessor_count = 0;
    char accessors[32][METHOD_MAX_LENGTH];
	WrenType accessor_type[32];
	char accessor_getter[METHOD_MAX_LENGTH];
    
    int  function_count = 0;
    char functions[16][METHOD_MAX_LENGTH];
    
    // Load the class into slot 0.
    wrenEnsureSlots(vm, 1);
    
    if ( !wrenHasVariable(vm, "script", "Script"))
    {
        fprintf(stderr, "Script class is missing");
        exit(-1);
    }
    
    // Get a handle to the Script class
    wrenGetVariable(vm, "script", "Script", 0);
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
                
				if (length > 0)
				{
					// truncate if necessary
					if (length > METHOD_MAX_LENGTH)
						length = METHOD_MAX_LENGTH;

					strncpy(methods[method_count], name, length);
					methods[method_count][length] = '\0';
					method_count++;
					if (method_count >= METHOD_MAX)
					{
						printf("Maximum functions reached\n");
						break;
					}
				}
            }
        }
    }

	printf("\n");
	// Get the handle to the script constructor
	WrenHandle* constructor = wrenMakeCallHandle(vm, "new()");

	// Create a script instance
	wrenSetSlotHandle(vm, 0, script_class);
	result = wrenCall(vm, constructor);

	// Check the result
	handle_result(result);

	// Get a handle to the new instance
	WrenHandle* script_instance = wrenGetSlotHandle(vm, 0);
    
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
			
			// If the method doesn't have parentheses - it is a getter
			bool is_getter = (strchr(methods[i], '(') == NULL);

            // If the method has an = sign then it is a setter
            bool is_setter = (strstr(methods[i], "=(") != NULL);

            if (is_setter)
			{
				// ignore and mark as processed
				methods[i][0] = '\0';
			}
			else if (is_getter)
			{
				// store getter function name
				size_t length = strlen(methods[i]);
				strncpy(accessors[accessor_count], methods[i], length);
				accessors[accessor_count][length] = '\0';
				
				// Set the script instance
				wrenSetSlotHandle(vm, 0, script_instance);

                // Get a handle to the getter function on the instance
				WrenHandle* accessor_handle = wrenMakeCallHandle(vm, methods[i]);

                // Call it to get the getter value
				result = wrenCall(vm, accessor_handle);

				// Check the result
				handle_result(result);

                // Get the type of value returned from the getter
				accessor_type[accessor_count] = wrenGetSlotType(vm, 0);
				accessor_count++;

				methods[i][0] = '\0';
			}
			else
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
    {
		printf("%s: Type: ", accessors[i]);
        type_to_string(accessor_type[i]);
    }
    
    printf("\nFunctions\n--------------\n");
    for (int i = 0; i < function_count; i++)
        printf("%s\n", functions[i]);
        
    wrenReleaseHandle(vm, script_class);
}

void test_foreign_class()
{
    WrenInterpretResult result;

    // Load the class into slot 0.
    wrenEnsureSlots(vm, 1);
    
    if ( !wrenHasVariable(vm, "script", "BehaviourComponent"))
    {
        fprintf(stderr, "Script class is missing");
        exit(-1);
    }
    
    // Get a handle to the Script class
    wrenGetVariable(vm, "script", "BehaviourComponent", 0);
    WrenHandle *script_class = wrenGetSlotHandle(vm, 0);

    printf("\n");
	// Get the handle to the script constructor
	WrenHandle* constructor = wrenMakeCallHandle(vm, "new()");
    WrenHandle* update = wrenMakeCallHandle(vm, "update(_)");
    WrenHandle* pos_setter = wrenMakeCallHandle(vm, "pos(_)");

	// Create a script instance
	wrenSetSlotHandle(vm, 0, script_class);
	result = wrenCall(vm, constructor);

	// Check the result
	handle_result(result);

	// Get a handle to the new instance
	WrenHandle* script_instance = wrenGetSlotHandle(vm, 0);
    
    // Set new vec3.x  parameter
    wrenSetSlotDouble(vm, 1, 0.5f);

    // Call Script.update
    result = wrenCall(vm, update);

    // Check the result
    handle_result(result);

    // TODO: Create an object, don't call the constructor 
    // Create a vec3 object
    // and set the pos field to the vec3 object via the setter

}

WrenForeignClassMethods bindForeignClass(WrenVM* vm, const char* module, const char* className)
{
    WrenForeignClassMethods methods;
    bind_vec3_class(vm, &methods, module, className);

    return methods;
}

WrenForeignMethodFn bindForeignMethod(WrenVM* vm,
                                      const char* moduleName,
                                      const char* className,
                                      bool isStatic,
                                      const char* signature)
{
    WrenForeignMethodFn func;
    func = bind_vec3_methods(vm, moduleName, className, isStatic, signature);

    return func;
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

    // Process foreign class testing
    test_foreign_class();

    wrenFreeVM(vm);

    return exitCode;
}


