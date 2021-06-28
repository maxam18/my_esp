idf_component_register(SRCS "me_ads111x.c"
                    INCLUDE_DIRS "include"
                    REQUIRES me_i2c
                    REQUIRES me_debug)
