#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=a
DEBUGGABLE_SUFFIX=
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/LIB_serial_uart.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=a
DEBUGGABLE_SUFFIX=
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/LIB_serial_uart.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/src/serial_uart_u1a.o ${OBJECTDIR}/src/serial_uart_u1b.o
POSSIBLE_DEPFILES=${OBJECTDIR}/src/serial_uart_u1a.o.d ${OBJECTDIR}/src/serial_uart_u1b.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/src/serial_uart_u1a.o ${OBJECTDIR}/src/serial_uart_u1b.o


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE} ${MAKE_OPTIONS} -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/LIB_serial_uart.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX795F512L
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/src/serial_uart_u1a.o: src/serial_uart_u1a.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/serial_uart_u1a.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/serial_uart_u1a.o.d" $(SILENT) -c  ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -mips16 -mno-float -I"src" -I"src/SRC_uC_Peripheral_Map" -I"src/INTERFACE_COM_PIC32MX795F512L_STD" -Os -Wall -MMD -MF "${OBJECTDIR}/src/serial_uart_u1a.o.d" -o ${OBJECTDIR}/src/serial_uart_u1a.o src/serial_uart_u1a.c  
	
${OBJECTDIR}/src/serial_uart_u1b.o: src/serial_uart_u1b.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/serial_uart_u1b.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/serial_uart_u1b.o.d" $(SILENT) -c  ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -mips16 -mno-float -I"src" -I"src/SRC_uC_Peripheral_Map" -I"src/INTERFACE_COM_PIC32MX795F512L_STD" -Os -Wall -MMD -MF "${OBJECTDIR}/src/serial_uart_u1b.o.d" -o ${OBJECTDIR}/src/serial_uart_u1b.o src/serial_uart_u1b.c  
	
else
${OBJECTDIR}/src/serial_uart_u1a.o: src/serial_uart_u1a.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/serial_uart_u1a.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/serial_uart_u1a.o.d" $(SILENT) -c  ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -mips16 -mno-float -I"src" -I"src/SRC_uC_Peripheral_Map" -I"src/INTERFACE_COM_PIC32MX795F512L_STD" -Os -Wall -MMD -MF "${OBJECTDIR}/src/serial_uart_u1a.o.d" -o ${OBJECTDIR}/src/serial_uart_u1a.o src/serial_uart_u1a.c  
	
${OBJECTDIR}/src/serial_uart_u1b.o: src/serial_uart_u1b.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/src 
	@${RM} ${OBJECTDIR}/src/serial_uart_u1b.o.d 
	@${FIXDEPS} "${OBJECTDIR}/src/serial_uart_u1b.o.d" $(SILENT) -c  ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -mips16 -mno-float -I"src" -I"src/SRC_uC_Peripheral_Map" -I"src/INTERFACE_COM_PIC32MX795F512L_STD" -Os -Wall -MMD -MF "${OBJECTDIR}/src/serial_uart_u1b.o.d" -o ${OBJECTDIR}/src/serial_uart_u1b.o src/serial_uart_u1b.c  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: archive
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/LIB_serial_uart.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_AR} $(MP_EXTRA_AR_PRE) r dist/${CND_CONF}/${IMAGE_TYPE}/LIB_serial_uart.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}    
else
dist/${CND_CONF}/${IMAGE_TYPE}/LIB_serial_uart.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_AR} $(MP_EXTRA_AR_PRE) r dist/${CND_CONF}/${IMAGE_TYPE}/LIB_serial_uart.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}    
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
