#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Build and launch CarlaUnreal."

USAGE_STRING="Usage: $0 [-h|--help] [--build] [--rebuild] [--launch] [--clean] [--hard-clean] [--opengl]"

REMOVE_INTERMEDIATE=false
HARD_CLEAN=false
BUILD_CARLA_UNREAL=false
LAUNCH_UNREAL_EDITOR=false
USE_CARSIM=false
USE_CHRONO=false
USE_PYTORCH=false
EDITOR_FLAGS=""
USE_HOUDINI=false

GDB=
RHI="-vulkan"

OPTS=`getopt -o h --long help,build,rebuild,launch,clean,hard-clean,gdb,opengl,carsim,pytorch,chrono,editor-flags: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --editor-flags )
      EDITOR_FLAGS=$2
      shift ;;
    --gdb )
      GDB="gdb --args";
      shift ;;
    --build )
      BUILD_CARLA_UNREAL=true;
      shift ;;
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_CARLA_UNREAL=true;
      shift ;;
    --launch )
      LAUNCH_UNREAL_EDITOR=true;
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;
      shift ;;
    --hard-clean )
      REMOVE_INTERMEDIATE=true;
      HARD_CLEAN=true;
      shift ;;
    --opengl )
      RHI="-opengl";
      shift ;;
    --carsim )
      USE_CARSIM=true;
      shift ;;
    --chrono )
      USE_CHRONO=true
      shift ;;
    --pytorch )
      USE_PYTORCH=true;
      shift ;;
    --with-houdini )
      USE_HOUDINI=true;
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      shift ;;
  esac
done

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

source $(dirname "$0")/Environment.sh

if [ ! -d "${CARLA_UE_ROOT}" ]; then
  fatal_error "CARLA_UE_ROOT is not defined, or points to a non-existant directory, please set this environment variable."
else
  log "Using Unreal Engine at '$CARLA_UE_ROOT'"
fi

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_CARLA_UNREAL} || ${LAUNCH_UNREAL_EDITOR}; }; then
  fatal_error "Nothing selected to be done."
fi

pushd "${CARLA_UNREAL_ROOT_FOLDER}" >/dev/null

# ==============================================================================
# -- Clean CarlaUnreal ------------------------------------------------------------
# ==============================================================================

if ${HARD_CLEAN} ; then

  if [ ! -f Makefile ]; then
    fatal_error "The project wasn't built before!"
  fi

  log "Doing a \"hard\" clean of the Unreal Engine project."

  make CarlaUnrealEditor ARGS=-clean

fi

if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  UNREAL_ENGINE_INTERMEDIATE_FOLDERS="Binaries Build Intermediate DerivedDataCache"

  rm -Rf ${UNREAL_ENGINE_INTERMEDIATE_FOLDERS}

  rm -f Makefile

  pushd "${CARLA_UNREAL_PLUGIN_ROOT_FOLDER}" >/dev/null

  rm -Rf ${UNREAL_ENGINE_INTERMEDIATE_FOLDERS}

  popd >/dev/null

fi

# ==============================================================================
# -- Download Houdini Plugin for Unreal Engine ---------------------------------
# ==============================================================================

HOUDINI_PLUGIN_REPO=https://github.com/sideeffects/HoudiniEngineForUnreal.git
HOUDINI_PLUGIN_PATH=Plugins/HoudiniEngine
HOUDINI_PLUGIN_BRANCH=Houdini19.5-Unreal5.00
HOUDINI_PATCH=${CARLA_UTIL_FOLDER}/Patches/houdini_patch.txt
if [[ ! -d ${HOUDINI_PLUGIN_PATH} ]] ; then
  git clone -b ${HOUDINI_PLUGIN_BRANCH} ${HOUDINI_PLUGIN_REPO} ${HOUDINI_PLUGIN_PATH}
  pushd ${HOUDINI_PLUGIN_PATH} >/dev/null
  git apply ${HOUDINI_PATCH}
  popd >/dev/null
fi

# ==============================================================================
# -- Build CarlaUnreal ------------------------------------------------------------
# ==============================================================================

if ${BUILD_CARLA_UNREAL} ; then

  OPTIONAL_MODULES_TEXT=""
  if ${USE_CARSIM} ; then
    python ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUnreal.uproject" -e
    OPTIONAL_MODULES_TEXT="CarSim ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  else
    python ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUnreal.uproject"
    OPTIONAL_MODULES_TEXT="CarSim OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  fi
  if ${USE_CHRONO} ; then
    OPTIONAL_MODULES_TEXT="Chrono ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  else
    OPTIONAL_MODULES_TEXT="Chrono OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  fi
  if ${USE_PYTORCH} ; then
    OPTIONAL_MODULES_TEXT="Pytorch ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  else
    OPTIONAL_MODULES_TEXT="Pytorch OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  fi
  echo ${OPTIONAL_MODULES_TEXT} > ${PWD}/Config/OptionalModules.ini

  if [ ! -f Makefile ]; then

    # This command fails sometimes but normally we can continue anyway.
    set +e
    log "Generate Unreal project files."
    ${CARLA_UE_ROOT}/GenerateProjectFiles.sh -project="${PWD}/CarlaUnreal.uproject" -game -engine -makefiles
    set -e

  fi

  log "Build CarlaUnreal project."
  make CarlaUnrealEditor

  #Providing the user with the ExportedMaps folder
  EXPORTED_MAPS="${CARLA_UNREAL_ROOT_FOLDER}/Content/Carla/ExportedMaps"
  mkdir -p "${EXPORTED_MAPS}"


fi

# ==============================================================================
# -- Launch UnrealEditor ----------------------------------------------------------
# ==============================================================================

if ${LAUNCH_UNREAL_EDITOR} ; then

  log "Launching UnrealEditor..."
  ${GDB} ${CARLA_UE_ROOT}/Engine/Binaries/Linux/UnrealEditor "${PWD}/CarlaUnreal.uproject" ${RHI} ${EDITOR_FLAGS}

else

  log "Success!"

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null
