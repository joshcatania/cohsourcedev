// Issue 36 Web Swing animation slice. The installer includes this file from
// the runtime player sequencer after resolving the local player source.

Move WEBSWING_AIRBORNE
	Interpolate 5
	Priority 20
	Type Male
		Anim MALE/SUPERJUMP 66 93
		TEnd
	Type Huge
		Anim HUGE/SUPERJUMP 66 93
		TEnd
	Type Fem
		Anim FEM/SUPERJUMP 66 93
		TEnd
	Member "<WEBSWING_ANIM>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires WEBSWING_AIRBORNE
	Flags Cycle
MEnd

Move WEBSWING_ATTACHED
	Interpolate 5
	Priority 20
	Type Male
		Anim MALE/FLY 25 81
		TEnd
	Type Huge
		Anim HUGE/FLOAT 1 50
		TEnd
	Type Fem
		Anim FEM/HOVER 57 106
		TEnd
	Member "<WEBSWING_ANIM>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires WEBSWING_ATTACHED
	Flags Cycle
MEnd

Move WEBSWING_DESCEND
	Interpolate 5
	Priority 22
	Type Male
		Anim MALE/FLYDOWN 1 226
		TEnd
	Type Huge
		Anim HUGE/FLYDOWN 1 226
		TEnd
	Type Fem
		Anim FEM/FLY_DOWN 1 226
		TEnd
	Member "<WEBSWING_ANIM>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_DESCEND"
	Flags Cycle
MEnd

Move WEBSWING_BOTTOM
	Interpolate 5
	Priority 22
	Type Male
		Anim MALE/FLOAT2 1 50
		TEnd
	Type Huge
		Anim HUGE/FLOAT 1 50
		TEnd
	Type Fem
		Anim FEM/FLY_READY 1 50
		TEnd
	Member "<WEBSWING_ANIM>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_BOTTOM"
	Flags Cycle
MEnd

Move WEBSWING_ASCEND
	Interpolate 5
	Priority 22
	Type Male
		Anim MALE/FLY 1 25
		TEnd
	Type Huge
		Anim HUGE/FLOAT 1 50
		TEnd
	Type Fem
		Anim FEM/HOVER 1 54
		TEnd
	Member "<WEBSWING_ANIM>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_ASCEND"
	Flags Cycle
MEnd
