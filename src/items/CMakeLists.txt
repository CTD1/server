PROJECT(items C)

SET(_TEST_FILES
xerewards.test.c
)

SET(_FILES
artrewards.c
demonseye.c
itemtypes.c
phoenixcompass.c
seed.c
speedsail.c
weapons.c
xerewards.c
)
FOREACH(_FILE ${_FILES})
    LIST(APPEND _SOURCES ${PROJECT_NAME}/${_FILE})
ENDFOREACH(_FILE)
SET(ITEMS_SRC ${_SOURCES} PARENT_SCOPE)
    
FOREACH(_FILE ${_TEST_FILES})
    LIST(APPEND _TESTS ${PROJECT_NAME}/${_FILE})
ENDFOREACH(_FILE)
SET(ITEMS_TESTS ${_TESTS} PARENT_SCOPE)
    
