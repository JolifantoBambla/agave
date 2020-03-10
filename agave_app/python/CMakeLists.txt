target_include_directories(agaveapp PUBLIC
	"${CMAKE_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}"
	${Boost_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    "pybind11"
)
target_sources(agaveapp PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/pyagave.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pyrenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pyrenderer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/RenderInterface.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ScriptServer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ScriptServer.h"
)
add_subdirectory(pybind11)

pybind11_add_module(pyagave MODULE
    "${CMAKE_CURRENT_SOURCE_DIR}/pyagavemodule.cpp"
    # "${CMAKE_CURRENT_SOURCE_DIR}/pyrenderer.cpp"
    # "${CMAKE_CURRENT_SOURCE_DIR}/pyrenderer.h"
    # "${CMAKE_CURRENT_SOURCE_DIR}/RenderInterface.h"
)
target_include_directories(pyagave PUBLIC
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    ${Boost_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    "pybind11"
)
target_link_libraries(pyagave PRIVATE
	# renderlib 
	# Qt5::Core Qt5::Gui Qt5::Network Qt5::WebSockets Qt5::Xml
	# ${TIFF_LIBRARIES}
	${Boost_LIBRARIES}
)
add_custom_command(TARGET pyagave POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pyagave> ${PROJECT_SOURCE_DIR}
)
