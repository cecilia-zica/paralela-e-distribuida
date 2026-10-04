#ifndef GAME_RULES_H
#define GAME_RULES_H

#define MAX_RULE_LEN 32

typedef struct {
    int birth[9];
    int survival[9];
} Rule;

void parse_rule(const char *text, Rule *rule);

#endif
