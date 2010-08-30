#ifndef EXECUTION_H
#define EXECUTION_H 1

#include "command.h"
#include "condition.h"
#include "execution_ctx.h"
#include "expression.h"
#include "ruleset.h"

#include "../uevent.h"
#include "../settings.h"

int ruleset_flags(struct ruleset_t *, struct uevent_t *);
int ruleset_execute(struct ruleset_t *, struct uevent_t *, struct settings_t *);

#endif /* EXECUTION_H */
