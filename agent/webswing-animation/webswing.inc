// Issue 36 Web Swing pose vocabulary V1. The installer includes this file
// from the runtime player sequencer after resolving the local player source.
//
// The five states reuse three weaponless, cross-skeleton source families:
// AIR_MA_IRONKICK's authored main/recovery split provides the directional
// extension, AIR_WINDUP_UPCUT's post range provides the raised-arm read, and
// JUMP_N_SMASH's post range provides the compact bottom pose. These are
// intentional subranges from existing authored moves, not filename-only picks.
//
// Ninja/Beast jump, apex, fall, hop, and post ranges were audited separately.
// Their authored tracks are Male-only; Parkour has a predictable state but no
// authored Parkour family in this source. They remain release follow-up
// candidates rather than cross-skeleton primary poses. WEBSWING_RELEASE is
// intentionally not added until a visual-only eligibility/state-bit path is
// available and the range passes a GUI audition.

Move WEBSWING_AIRBORNE
	Interpolate 5
	Priority 20
	Type Male
		Anim MALE/AIR_MA_IRONKICK 34 84
		TEnd
	Type Huge
		Anim HUGE/AIR_MA_IRONKICK 34 84
		TEnd
	Type Fem
		Anim FEM/AIR_MA_IRONKICK 34 84
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
		Anim MALE/AIR_WINDUP_UPCUT 65 89
		TEnd
	Type Huge
		Anim HUGE/AIR_WINDUP_UPCUT 65 89
		TEnd
	Type Fem
		Anim FEM/AIR_WINDUP_UPCUT 65 89
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
		Anim MALE/AIR_MA_IRONKICK 84 120
		TEnd
	Type Huge
		Anim HUGE/AIR_MA_IRONKICK 84 120
		TEnd
	Type Fem
		Anim FEM/AIR_MA_IRONKICK 84 120
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
		Anim MALE/JUMP_N_SMASH 98 124
		TEnd
	Type Huge
		Anim HUGE/JUMP_N_SMASH 98 124
		TEnd
	Type Fem
		Anim FEM/JUMP_N_SMASH 98 124
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
		Anim MALE/AIR_WINDUP_UPCUT 65 89
		TEnd
	Type Huge
		Anim HUGE/AIR_WINDUP_UPCUT 65 89
		TEnd
	Type Fem
		Anim FEM/AIR_WINDUP_UPCUT 65 89
		TEnd
	Member "<WEBSWING_ANIM>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_ASCEND"
	Flags Cycle
MEnd
