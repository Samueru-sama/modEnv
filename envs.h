#ifndef ENVS_H
#define ENVS_H

#include <stdbool.h>

typedef bool (*ConditionFunc)(void);

typedef struct {
    const char *env;
    const char *value;
    ConditionFunc condition;
} ConditionalEnv;

extern ConditionalEnv conditional_envs[];

#endif
