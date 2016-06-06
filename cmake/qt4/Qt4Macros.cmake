# This file is included by FindQt4.cmake, don't include it directly.

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)


######################################
#
#       Macros for building Qt files
#
######################################


macro (QT4_EXTRACT_OPTIONS _qt4_files _qt4_options)
  set(${_qt4_files})
  set(${_qt4_options})
  set(_QT4_DOING_OPTIONS FALSE)
  foreach(_currentArg ${ARGN})
    if ("${_currentArg}" STREQUAL "OPTIONS")
      set(_QT4_DOING_OPTIONS TRUE)
    else ()
      if(_QT4_DOING_OPTIONS)
        list(APPEND ${_qt4_options} "${_currentArg}")
      else()
        list(APPEND ${_qt4_files} "${_currentArg}")
      endif()
    endif ()
  endforeach()
endmacro ()


# macro used to create the names of output files preserving relative dirs
macro (QT4_MAKE_OUTPUT_FILE infile prefix ext outfile )
  string(LENGTH ${CMAKE_CURRENT_BINARY_DIR} _binlength)
  string(LENGTH ${infile} _infileLength)
  set(_checkinfile ${CMAKE_CURRENT_SOURCE_DIR})
  if(_infileLength GREATER _binlength)
    string(SUBSTRING "${infile}" 0 ${_binlength} _checkinfile)
    if(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
      file(RELATIVE_PATH rel ${CMAKE_CURRENT_BINARY_DIR} ${infile})
    else()
      file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
    endif()
  else()
    file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
  endif()
  if(WIN32 AND rel MATCHES "^[a-zA-Z]:") # absolute path
    string(REGEX REPLACE "^([a-zA-Z]):(.*)$" "\\1_\\2" rel "${rel}")
  endif()
  set(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${rel}")
  string(REPLACE ".." "__" _outfile ${_outfile})
  get_filename_component(outpath ${_outfile} PATH)
  get_filename_component(_outfile ${_outfile} NAME_WE)
  file(MAKE_DIRECTORY ${outpath})
  set(${outfile} ${outpath}/${prefix}${_outfile}.${ext})
endmacro ()


macro (QT4_GET_MOC_FLAGS _moc_flags)
  set(${_moc_flags})
  get_directory_property(_inc_DIRS INCLUDE_DIRECTORIES)

  foreach(_current ${_inc_DIRS})
    if("${_current}" MATCHES "\\.framework/?$")
      string(REGEX REPLACE "/[^/]+\\.framework" "" framework_path "${_current}")
      set(${_moc_flags} ${${_moc_flags}} "-F${framework_path}")
    else()
      set(${_moc_flags} ${${_moc_flags}} "-I${_current}")
    endif()
  endforeach()

  get_directory_property(_defines COMPILE_DEFINITIONS)
  foreach(_current ${_defines})
    set(${_moc_flags} ${${_moc_flags}} "-D${_current}")
  endforeach()

  if(Q_WS_WIN)
    set(${_moc_flags} ${${_moc_flags}} -DWIN32)
  endif()

endmacro()


# helper macro to set up a moc rule
macro (QT4_CREATE_MOC_COMMAND infile outfile moc_flags moc_options)
  # For Windows, create a parameters file to work around command line length limit
  if (WIN32)
    # Pass the parameters in a file.  Set the working directory to
    # be that containing the parameters file and reference it by
    # just the file name.  This is necessary because the moc tool on
    # MinGW builds does not seem to handle spaces in the path to the
    # file given with the @ syntax.
    get_filename_component(_moc_outfile_name "${outfile}" NAME)
    get_filename_component(_moc_outfile_dir "${outfile}" PATH)
    if(_moc_outfile_dir)
      set(_moc_working_dir WORKING_DIRECTORY ${_moc_outfile_dir})
    endif()
    set (_moc_parameters_file ${outfile}_parameters)
    set (_moc_parameters ${moc_flags} ${moc_options} -o "${outfile}" "${infile}")
    string (REPLACE ";" "\n" _moc_parameters "${_moc_parameters}")
    file (WRITE ${_moc_parameters_file} "${_moc_parameters}")
    add_custom_command(OUTPUT ${outfile}
                       COMMAND ${QT_MOC_EXECUTABLE} @${_moc_outfile_name}_parameters
                       DEPENDS ${infile}
                       ${_moc_working_dir}
                       VERBATIM)
  else ()
    add_custom_command(OUTPUT ${outfile}
                       COMMAND ${QT_MOC_EXECUTABLE}
                       ARGS ${moc_flags} ${moc_options} -o ${outfile} ${infile}
                       DEPENDS ${infile} VERBATIM)
  endif ()
endmacro ()


macro (QT4_GENERATE_MOC infile outfile )
# get include dirs and flags
   QT4_GET_MOC_FLAGS(moc_flags)
   get_filename_component(abs_infile ${infile} ABSOLUTE)
   set(_outfile "${outfile}")
   if(NOT IS_ABSOLUTE "${outfile}")
     set(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${outfile}")
   endif()
   QT4_CREATE_MOC_COMMAND(${abs_infile} ${_outfile} "${moc_flags}" "")
   set_source_files_properties(${outfile} PROPERTIES SKIP_AUTOMOC TRUE)  # dont run automoc on this file
endmacro ()


# QT4_WRAP_CPP(outfiles inputfile ... )

macro (QT4_WRAP_CPP outfiles )
  # get include dirs
  QT4_GET_MOC_FLAGS(moc_flags)
  QT4_EXTRACT_OPTIONS(moc_files moc_options ${ARGN})

  foreach (it ${moc_files})
    get_filename_component(it ${it} ABSOLUTE)
    QT4_MAKE_OUTPUT_FILE(${it} moc_ cxx outfile)
    QT4_CREATE_MOC_COMMAND(${it} ${outfile} "${moc_flags}" "${moc_options}")
    set(${outfiles} ${${outfiles}} ${outfile})
  endforeach()

endmacro ()


# QT4_WRAP_UI(outfiles inputfile ... )

macro (QT4_WRAP_UI outfiles )
  QT4_EXTRACT_OPTIONS(ui_files ui_options ${ARGN})

  foreach (it ${ui_files})
    get_filename_component(outfile ${it} NAME_WE)
    get_filename_component(infile ${it} ABSOLUTE)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/ui_${outfile}.h)
    add_custom_command(OUTPUT ${outfile}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS ${ui_options} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile} VERBATIM)
    set(${outfiles} ${${outfiles}} ${outfile})
  endforeach ()

endmacro ()


# QT4_ADD_RESOURCES(outfiles inputfile ... )

macro (QT4_ADD_RESOURCES outfiles )
  QT4_EXTRACT_OPTIONS(rcc_files rcc_options ${ARGN})

  foreach (it ${rcc_files})
    get_filename_component(outfilename ${it} NAME_WE)
    get_filename_component(infile ${it} ABSOLUTE)
    get_filename_component(rc_path ${infile} PATH)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/qrc_${outfilename}.cxx)

    set(_RC_DEPENDS)
    if(EXISTS "${infile}")
      #  parse file for dependencies
      #  all files are absolute paths or relative to the location of the qrc file
      file(READ "${infile}" _RC_FILE_CONTENTS)
      string(REGEX MATCHALL "<file[^<]+" _RC_FILES "${_RC_FILE_CONTENTS}")
      foreach(_RC_FILE ${_RC_FILES})
        string(REGEX REPLACE "^<file[^>]*>" "" _RC_FILE "${_RC_FILE}")
        if(NOT IS_ABSOLUTE "${_RC_FILE}")
          set(_RC_FILE "${rc_path}/${_RC_FILE}")
        endif()
        set(_RC_DEPENDS ${_RC_DEPENDS} "${_RC_FILE}")
      endforeach()
      unset(_RC_FILES)
      unset(_RC_FILE_CONTENTS)
      # Since this cmake macro is doing the dependency scanning for these files,
      # let's make a configured file and add it as a dependency so cmake is run
      # again when dependencies need to be recomputed.
      QT4_MAKE_OUTPUT_FILE("${infile}" "" "qrc.depends" out_depends)
      configure_file("${infile}" "${out_depends}" COPYONLY)
    else()
      # The .qrc file does not exist (yet). Let's add a dependency and hope
      # that it will be generated later
      set(out_depends)
    endif()

    add_custom_command(OUTPUT ${outfile}
      COMMAND ${QT_RCC_EXECUTABLE}
      ARGS ${rcc_options} -name ${outfilename} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile}
      DEPENDS ${_RC_DEPENDS} "${out_depends}" VERBATIM)
    set(${outfiles} ${${outfiles}} ${outfile})
  endforeach ()

endmacro ()


macro(QT4_ADD_DBUS_INTERFACE _sources _interface _basename)
  get_filename_component(_infile ${_interface} ABSOLUTE)
  set(_header "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h")
  set(_impl   "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp")
  set(_moc    "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc")

  get_source_file_property(_nonamespace ${_interface} NO_NAMESPACE)
  if(_nonamespace)
    set(_params -N -m)
  else()
    set(_params -m)
  endif()

  get_source_file_property(_classname ${_interface} CLASSNAME)
  if(_classname)
    set(_params ${_params} -c ${_classname})
  endif()

  get_source_file_property(_include ${_interface} INCLUDE)
  if(_include)
    set(_params ${_params} -i ${_include})
  endif()

  add_custom_command(OUTPUT "${_impl}" "${_header}"
      COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} ${_params} -p ${_basename} ${_infile}
      DEPENDS ${_infile} VERBATIM)

  set_source_files_properties("${_impl}" PROPERTIES SKIP_AUTOMOC TRUE)

  QT4_GENERATE_MOC("${_header}" "${_moc}")

  list(APPEND ${_sources} "${_impl}" "${_header}" "${_moc}")
  MACRO_ADD_FILE_DEPENDENCIES("${_impl}" "${_moc}")

endmacro()


macro(QT4_ADD_DBUS_INTERFACES _sources)
  foreach (_current_FILE ${ARGN})
    get_filename_component(_infile ${_current_FILE} ABSOLUTE)
    get_filename_component(_basename ${_current_FILE} NAME)
    # get the part before the ".xml" suffix
    string(TOLOWER ${_basename} _basename)
    string(REGEX REPLACE "(.*\\.)?([^\\.]+)\\.xml" "\\2" _basename ${_basename})
    QT4_ADD_DBUS_INTERFACE(${_sources} ${_infile} ${_basename}interface)
  endforeach ()
endmacro()


macro(QT4_GENERATE_DBUS_INTERFACE _header) # _customName OPTIONS -some -options )
  QT4_EXTRACT_OPTIONS(_customName _qt4_dbus_options ${ARGN})

  get_filename_component(_in_file ${_header} ABSOLUTE)
  get_filename_component(_basename ${_header} NAME_WE)

  if (_customName)
    if (IS_ABSOLUTE ${_customName})
      get_filename_component(_containingDir ${_customName} PATH)
      if (NOT EXISTS ${_containingDir})
        file(MAKE_DIRECTORY "${_containingDir}")
      endif()
      set(_target ${_customName})
    else()
      set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_customName})
    endif()
  else ()
    set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.xml)
  endif ()

  add_custom_command(OUTPUT ${_target}
      COMMAND ${QT_DBUSCPP2XML_EXECUTABLE} ${_qt4_dbus_options} ${_in_file} -o ${_target}
      DEPENDS ${_in_file} VERBATIM
  )
endmacro()


macro(QT4_ADD_DBUS_ADAPTOR _sources _xml_file _include _parentClass) # _optionalBasename _optionalClassName)
  get_filename_component(_infile ${_xml_file} ABSOLUTE)

  set(_optionalBasename "${ARGV4}")
  if (_optionalBasename)
    set(_basename ${_optionalBasename} )
  else ()
    string(REGEX REPLACE "(.*[/\\.])?([^\\.]+)\\.xml" "\\2adaptor" _basename ${_infile})
    string(TOLOWER ${_basename} _basename)
  endif ()

  set(_optionalClassName "${ARGV5}")
  set(_header "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h")
  set(_impl   "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp")
  set(_moc    "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc")

  if(_optionalClassName)
    add_custom_command(OUTPUT "${_impl}" "${_header}"
       COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} -m -a ${_basename} -c ${_optionalClassName} -i ${_include} -l ${_parentClass} ${_infile}
       DEPENDS ${_infile} VERBATIM
    )
  else()
    add_custom_command(OUTPUT "${_impl}" "${_header}"
       COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} -m -a ${_basename} -i ${_include} -l ${_parentClass} ${_infile}
       DEPENDS ${_infile} VERBATIM
     )
  endif()

  QT4_GENERATE_MOC("${_header}" "${_moc}")
  set_source_files_properties("${_impl}" PROPERTIES SKIP_AUTOMOC TRUE)
  MACRO_ADD_FILE_DEPENDENCIES("${_impl}" "${_moc}")

  list(APPEND ${_sources} "${_impl}" "${_header}" "${_moc}")
endmacro()


macro(QT4_AUTOMOC)
  QT4_GET_MOC_FLAGS(_moc_INCS)

  set(_matching_FILES )
  foreach (_current_FILE ${ARGN})

    get_filename_component(_abs_FILE ${_current_FILE} ABSOLUTE)
    # if "SKIP_AUTOMOC" is set to true, we will not handle this file here.
    # This is required to make uic work correctly:
    # we need to add generated .cpp files to the sources (to compile them),
    # but we cannot let automoc handle them, as the .cpp files don't exist yet when
    # cmake is run for the very first time on them -> however the .cpp files might
    # exist at a later run. at that time we need to skip them, so that we don't add two
    # different rules for the same moc file
    get_source_file_property(_skip ${_abs_FILE} SKIP_AUTOMOC)

    if ( NOT _skip AND EXISTS ${_abs_FILE} )

      file(READ ${_abs_FILE} _contents)

      get_filename_component(_abs_PATH ${_abs_FILE} PATH)

      string(REGEX MATCHALL "# *include +[^ ]+\\.moc[\">]" _match "${_contents}")
      if(_match)
        foreach (_current_MOC_INC ${_match})
          string(REGEX MATCH "[^ <\"]+\\.moc" _current_MOC "${_current_MOC_INC}")

          get_filename_component(_basename ${_current_MOC} NAME_WE)
          if(EXISTS ${_abs_PATH}/${_basename}.hpp)
            set(_header ${_abs_PATH}/${_basename}.hpp)
          else()
            set(_header ${_abs_PATH}/${_basename}.h)
          endif()
          set(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_current_MOC})
          QT4_CREATE_MOC_COMMAND(${_header} ${_moc} "${_moc_INCS}" "")
          MACRO_ADD_FILE_DEPENDENCIES(${_abs_FILE} ${_moc})
        endforeach ()
      endif()
    endif ()
  endforeach ()
endmacro()


macro(QT4_CREATE_TRANSLATION _qm_files)
   QT4_EXTRACT_OPTIONS(_lupdate_files _lupdate_options ${ARGN})
   set(_my_sources)
   set(_my_dirs)
   set(_my_tsfiles)
   set(_ts_pro)
   foreach (_file ${_lupdate_files})
     get_filename_component(_ext ${_file} EXT)
     get_filename_component(_abs_FILE ${_file} ABSOLUTE)
     if(_ext MATCHES "ts")
       list(APPEND _my_tsfiles ${_abs_FILE})
     else()
       if(NOT _ext)
         list(APPEND _my_dirs ${_abs_FILE})
       else()
         list(APPEND _my_sources ${_abs_FILE})
       endif()
     endif()
   endforeach()
   foreach(_ts_file ${_my_tsfiles})
     if(_my_sources)
       # make a .pro file to call lupdate on, so we don't make our commands too
       # long for some systems
       get_filename_component(_ts_name ${_ts_file} NAME_WE)
       set(_ts_pro ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${_ts_name}_lupdate.pro)
       set(_pro_srcs)
       foreach(_pro_src ${_my_sources})
         set(_pro_srcs "${_pro_srcs} \"${_pro_src}\"")
       endforeach()
       set(_pro_includes)
       get_directory_property(_inc_DIRS INCLUDE_DIRECTORIES)
       foreach(_pro_include ${_inc_DIRS})
         get_filename_component(_abs_include "${_pro_include}" ABSOLUTE)
         set(_pro_includes "${_pro_includes} \"${_abs_include}\"")
       endforeach()
       file(WRITE ${_ts_pro} "SOURCES = ${_pro_srcs}\nINCLUDEPATH = ${_pro_includes}\n")
     endif()
     add_custom_command(OUTPUT ${_ts_file}
        COMMAND ${QT_LUPDATE_EXECUTABLE}
        ARGS ${_lupdate_options} ${_ts_pro} ${_my_dirs} -ts ${_ts_file}
        DEPENDS ${_my_sources} ${_ts_pro} VERBATIM)
   endforeach()
   QT4_ADD_TRANSLATION(${_qm_files} ${_my_tsfiles})
endmacro()


macro(QT4_ADD_TRANSLATION _qm_files)
  foreach (_current_FILE ${ARGN})
    get_filename_component(_abs_FILE ${_current_FILE} ABSOLUTE)
    get_filename_component(qm ${_abs_FILE} NAME_WE)
    get_source_file_property(output_location ${_abs_FILE} OUTPUT_LOCATION)
    if(output_location)
      file(MAKE_DIRECTORY "${output_location}")
      set(qm "${output_location}/${qm}.qm")
    else()
      set(qm "${CMAKE_CURRENT_BINARY_DIR}/${qm}.qm")
    endif()

    add_custom_command(OUTPUT ${qm}
       COMMAND ${QT_LRELEASE_EXECUTABLE}
       ARGS ${_abs_FILE} -qm ${qm}
       DEPENDS ${_abs_FILE} VERBATIM
    )
    set(${_qm_files} ${${_qm_files}} ${qm})
  endforeach ()
endmacro()

function(qt4_use_modules _target _link_type)
  if ("${_link_type}" STREQUAL "LINK_PUBLIC" OR "${_link_type}" STREQUAL "LINK_PRIVATE")
    set(modules ${ARGN})
    set(link_type ${_link_type})
  else()
    set(modules ${_link_type} ${ARGN})
  endif()
  foreach(_module ${modules})
    string(TOUPPER ${_module} _ucmodule)
    if (NOT QT_QT${_ucmodule}_FOUND)
      message(FATAL_ERROR "Can not use \"${_module}\" module which has not yet been found.")
    endif()
    if ("${_ucmodule}" STREQUAL "MAIN")
      message(FATAL_ERROR "Can not use \"${_module}\" module with qt4_use_modules.")
    endif()
    target_link_libraries(${_target} ${link_type} ${QT_QT${_ucmodule}_LIBRARY})
    set_property(TARGET ${_target} APPEND PROPERTY INCLUDE_DIRECTORIES ${QT_QT${_ucmodule}_INCLUDE_DIR} ${QT_HEADERS_DIR} ${QT_MKSPECS_DIR}/default)
    set_property(TARGET ${_target} APPEND PROPERTY COMPILE_DEFINITIONS ${QT_QT${_ucmodule}_COMPILE_DEFINITIONS})
  endforeach()
endfunction()
