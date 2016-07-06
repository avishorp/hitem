/*
 * statedef.h
 *
 *  Created on: Jul 6, 2016
 *      Author: local
 */

#ifndef STATEDEF_H_
#define STATEDEF_H_


struct state_t;
typedef const struct state_t* pState_t;
typedef pState_t (*stateHandler_t)();

typedef struct state_t {
	unsigned long stateSigniture;
	const char* stateName;
	stateHandler_t stateHandler;
} state_t;


#define STATE_HANDLER(st) static pState_t _StateHandle_ ## st()

#define STATE_SIGNITURE (0x849fc99a)

#define IS_VALID(s) (s->stateSigniture == STATE_SIGNITURE)

#define DEF_STATE(st) \
	STATE_HANDLER(st); \
	state_t __STATE_ ## st = { STATE_SIGNITURE, #st, &_StateHandle_ ## st }; \
	static const pState_t STATE_ ## st = &__STATE_ ## st;


#endif /* STATEDEF_H_ */
