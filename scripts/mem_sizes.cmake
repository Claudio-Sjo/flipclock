
# Retrieve the value of a symbol from SYMBOLS variable
function(get_symbol_val SYM)
    string(REGEX REPLACE "^.*\n([0-9]+) [AB] ${SYM}\n.*$" "\\1" VAL ${SYMBOLS})
    string(LENGTH ${VAL} L)
    if( L GREATER 10)
        set(VAL 1)
    endif()
    math(EXPR VAL ${VAL} OUTPUT_FORMAT DECIMAL )
    set(${SYM} ${VAL} PARENT_SCOPE)
endfunction()

# Rgith align passed string
function(right STR WIDTH)
    string(LENGTH ${${STR}} LEN)
    math(EXPR LEN "${WIDTH}-${LEN}")
    string(REPEAT " " ${LEN} SPACES)
    string(PREPEND ${STR} ${SPACES})
    set(${STR} ${${STR}} PARENT_SCOPE)
endfunction()


# Fill in symobls list
execute_process(
    COMMAND llvm-nm -radix=d ${ELF_FILE}
    OUTPUT_VARIABLE SYMBOLS
)
# Header
message("\nType       Size     Used")

foreach(MEM_STR "Flash    " "DTCM_RAM " "OC_NC_RAM" "SDRAM    ")
    # Find the linker defined symobls
    string(STRIP ${MEM_STR} MEM_TYPE)
    get_symbol_val(_${MEM_TYPE}_used)
    get_symbol_val(__size_${MEM_TYPE})
    # Calc 10000 + percentage * 10
    math(EXPR MEM_PC "10000 + (${_${MEM_TYPE}_used} * 1000) / ${__size_${MEM_TYPE}}" )
    math(EXPR __size_${MEM_TYPE} "${__size_${MEM_TYPE}}/1024" )
    # Integer part of percentage
    string(SUBSTRING ${MEM_PC} 2 2 INT_PC )
    math(EXPR INT_PC ${INT_PC} OUTPUT_FORMAT DECIMAL )
    # Decimal part of percentage
    string(SUBSTRING ${MEM_PC} 4 1 DEC_PC )
    # Right align numbers
    right(__size_${MEM_TYPE} 6)
    right(_${MEM_TYPE}_used 8)
    right(INT_PC 4)
    message("${MEM_STR}${__size_${MEM_TYPE}}k${_${MEM_TYPE}_used}${INT_PC}.${DEC_PC}%")
endforeach()
message("")