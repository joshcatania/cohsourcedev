// Issue 36 Web Swing — Male ascend hold via Mixamo 30-40 + frame-40 Scale 0
// V2 remains Male ATTACHED/DESCEND. Mixamo bottom 18-22 stays. Generic
// WEBSWING_ASCEND is the safe V1 fallback for Fem/Huge and for Male when
// helper bits are absent. The helper moves implement the one-shot + hold
// semantic without touching Fem/Huge, physics, or exporter math.

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
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_ASCEND>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires WEBSWING_AIRBORNE
	Flags Cycle
MEnd

Move WEBSWING_ATTACHED
	Interpolate 5
	Priority 20
	Type Male
		Anim MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2 1 30
	TEnd
	Type Huge
		Anim HUGE/COHSOURCEDEV_WEBSWING_STRETCH 1 30
	TEnd
	Type Fem
		Anim FEM/COHSOURCEDEV_WEBSWING_STRETCH 1 30
	TEnd
	Member "<WEBSWING_ANIM>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_ASCEND>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires WEBSWING_ATTACHED
	Flags Cycle
MEnd

Move WEBSWING_DESCEND
	Interpolate 5
	Priority 22
	Type Male
		Anim MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2 1 30
	TEnd
	Type Huge
		Anim HUGE/COHSOURCEDEV_WEBSWING_STRETCH 1 30
	TEnd
	Type Fem
		Anim FEM/COHSOURCEDEV_WEBSWING_STRETCH 1 30
	TEnd
	Member "<WEBSWING_ANIM>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_ASCEND>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_DESCEND"
	Flags Cycle
MEnd

Move WEBSWING_BOTTOM
	Interpolate 5
	Priority 22
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM 1 5
	TEnd
	Type Huge
		Anim HUGE/COHSOURCEDEV_WEBSWING_TUCK 1 24
	TEnd
	Type Fem
		Anim FEM/COHSOURCEDEV_WEBSWING_TUCK 1 24
	TEnd
	Member "<WEBSWING_ANIM>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_ASCEND>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_BOTTOM"
	Flags Cycle
MEnd

Move WEBSWING_ASCEND
	Interpolate 5
	Priority 22
	Type Male
		Anim MALE/COHSOURCEDEV_WEBSWING_ASCEND 1 28
	TEnd
	Type Huge
		Anim HUGE/COHSOURCEDEV_WEBSWING_ASCEND 1 28
	TEnd
	Type Fem
		Anim FEM/COHSOURCEDEV_WEBSWING_ASCEND 1 28
	TEnd
	Member "<WEBSWING_ANIM>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_ASCEND"
	Flags Cycle
MEnd

Move WEBSWING_ASCEND_MALE_START
	Interpolate 5
	Priority 23
	NextMove WEBSWING_ASCEND_MALE_HOLD
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 30 40
	TEnd
	Member "<WEBSWING_MALE_ASCEND>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_ASCEND", "WEBSWING_MALE", "WEBSWING_ASCEND_MALE_ENTER"
MEnd

Move WEBSWING_ASCEND_MALE_HOLD
	Interpolate 5
	Priority 23
	Scale 0
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 40 60
	TEnd
	Member "<WEBSWING_MALE_ASCEND>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_ASCEND", "WEBSWING_MALE"
	Flags Cycle
MEnd

// webswinganim 3 — MALE_FULL_CORRECTED. AIRBORNE+ATTACHED is the private
// mode discriminator; WEBSWING_ASCEND_MALE_ENTER is pulsed on every mode-3
// phase entry. Phase-specific moves outrank the generic attached helpers so
// their shared ATTACHED requirement cannot make both families alternate.
// They intentionally are not MOVEIRQ members: stock FFLY_PREFALL interrupts
// that group even at lower priority, which otherwise makes authored moves and
// the stock fall move replace each other every sequencer update. Cycle HOLDs
// still fall back through their NextMove when their Web Swing requirements
// clear, while STARTs finish their short authored window before doing so.
// Each authored phase START is itself the cycle owner. It remains selected
// until the controller changes phase, avoiding the START -> HOLD -> stock
// fallback race that let READY/profile moves replace the swing midair.

// V2 web-cycle choreography. Ground activation plays the complete authored
// crouch -> one-hand shot -> two-hand catch/pull clip. Repeated cycles first
// play the complete ground-launch performance forward again while the old
// line retracts and the replacement fires: planted preparation -> raised
// wrist -> leg kick-back -> tuck. The repeat is stretched across one full
// assisted arc instead of reaching its tuck halfway through and dwelling
// there. A moving tuck tail covers only unusually long arcs. A legitimate new
// assisted cycle may replace a still-finishing initial launch; no ordinary
// phase threshold can. Physics stays phase-driven; only the initial street
// boost waits for its web to visibly attach. No V2 HOLD is a static pose.

Move WEBSWING_V2_GROUND_LAUNCH_START
	Interpolate 4
	Priority 30
	NextMove WEBSWING_V2_GROUND_LAUNCH_HOLD
	Type Male
		Anim MALE/COHSOURCEDEV_WEBSWING_GROUND_LAUNCH_V2 1 62
	TEnd
	Member "<WEBSWING_V2_GROUND_LAUNCH>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_FULL_CORRECTED>", "<WEBSWING_V2_GROUND_LAUNCH>", "<WEBSWING_V2_RETRACT>", "<WEBSWING_V2_SHOOT>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_MALE", "WEBSWING_SHOOT", "WEBSWING_GROUND_LAUNCH", "WEBSWING_ASCEND_MALE_ENTER"
MEnd

Move WEBSWING_V2_GROUND_LAUNCH_HOLD
	Interpolate 4
	Priority 29
	Scale 0.45
	Type Male
		Anim MALE/COHSOURCEDEV_WEBSWING_GROUND_LAUNCH_V2 54 62
	TEnd
	Member "<WEBSWING_V2_GROUND_LAUNCH>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_MALE", "WEBSWING_GROUND_LAUNCH"
	Flags Cycle
MEnd

Move WEBSWING_V2_RETRACT_START
	Interpolate 2
	Priority 29
	Scale 0.65
	NextMove WEBSWING_V2_RETRACT_HOLD
	Type Male
		Anim MALE/COHSOURCEDEV_WEBSWING_GROUND_LAUNCH_V2 1 62
	TEnd
	Member "<WEBSWING_V2_RETRACT>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_FULL_CORRECTED>", "<WEBSWING_V2_GROUND_LAUNCH>", "<WEBSWING_V2_RETRACT>", "<WEBSWING_V2_SHOOT>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_MALE", "WEBSWING_RETRACT", "WEBSWING_ASCEND_MALE_ENTER"
MEnd

Move WEBSWING_V2_RETRACT_HOLD
	Interpolate 2
	Priority 28
	Scale 0.5
	Type Male
		Anim MALE/COHSOURCEDEV_WEBSWING_GROUND_LAUNCH_V2 54 62
	TEnd
	Member "<WEBSWING_V2_RETRACT>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_MALE", "WEBSWING_RETRACT"
	Flags Cycle
MEnd

Move WEBSWING_V2_SHOOT_START
	Interpolate 3
	Priority 29
	Type Male
		Anim MALE/COHSOURCEDEV_WEBSWING_SHOOT_V2 1 26
	TEnd
	Member "<WEBSWING_V2_SHOOT>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_FULL_CORRECTED>", "<WEBSWING_V2_RETRACT>", "<WEBSWING_V2_SHOOT>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_MALE", "WEBSWING_SHOOT"
	Flags Cycle
MEnd

Move WEBSWING_V2_SHOOT_HOLD
	Interpolate 3
	Priority 28
	Scale 0.5
	Type Male
		Anim MALE/COHSOURCEDEV_WEBSWING_SHOOT_V2 21 26
	TEnd
	Member "<WEBSWING_V2_SHOOT>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_MALE", "WEBSWING_SHOOT"
	Flags Cycle
MEnd

Move WEBSWING_FULL_ATTACHED_START
	Interpolate 5
	Priority 25
	Scale 0.35
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL 1 8
	TEnd
	Member "<WEBSWING_MALE_FULL_CORRECTED>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_FULL_CORRECTED>", "<WEBSWING_V2_RETRACT>", "<WEBSWING_V2_SHOOT>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_MALE"
	Flags Cycle
MEnd

Move WEBSWING_FULL_ATTACHED_HOLD
	Interpolate 5
	Priority 24
	Scale 0.35
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL 1 8
	TEnd
	Member "<WEBSWING_MALE_FULL_CORRECTED>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_MALE"
	Flags Cycle
MEnd

Move WEBSWING_FULL_DESCEND_START
	Interpolate 5
	Priority 26
	Scale 0.22
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL 9 17
	TEnd
	Member "<WEBSWING_MALE_FULL_CORRECTED>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_FULL_CORRECTED>", "<WEBSWING_V2_RETRACT>", "<WEBSWING_V2_SHOOT>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_DESCEND", "WEBSWING_MALE"
	Flags Cycle
MEnd

Move WEBSWING_FULL_DESCEND_HOLD
	Interpolate 5
	Priority 25
	Scale 0.22
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL 9 17
	TEnd
	Member "<WEBSWING_MALE_FULL_CORRECTED>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_DESCEND", "WEBSWING_MALE"
	Flags Cycle
MEnd

Move WEBSWING_FULL_BOTTOM_START
	Interpolate 5
	Priority 26
	Scale 0.25
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL 18 22
	TEnd
	Member "<WEBSWING_MALE_FULL_CORRECTED>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_FULL_CORRECTED>", "<WEBSWING_V2_RETRACT>", "<WEBSWING_V2_SHOOT>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_BOTTOM", "WEBSWING_MALE"
	Flags Cycle
MEnd

Move WEBSWING_FULL_BOTTOM_HOLD
	Interpolate 5
	Priority 25
	Scale 0.25
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL 18 22
	TEnd
	Member "<WEBSWING_MALE_FULL_CORRECTED>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_BOTTOM", "WEBSWING_MALE"
	Flags Cycle
MEnd

Move WEBSWING_FULL_ASCEND_START
	Interpolate 5
	Priority 26
	Scale 0.32
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL 23 40
	TEnd
	Member "<WEBSWING_MALE_FULL_CORRECTED>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<WEBSWING_MALE_FULL_CORRECTED>", "<WEBSWING_V2_SHOOT>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_ASCEND", "WEBSWING_MALE"
	Flags Cycle
MEnd

Move WEBSWING_FULL_ASCEND_HOLD
	Interpolate 5
	Priority 25
	Scale 0.32
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL 23 40
	TEnd
	Member "<WEBSWING_MALE_FULL_CORRECTED>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_AIRBORNE", "WEBSWING_ATTACHED", "WEBSWING_ASCEND", "WEBSWING_MALE"
	Flags Cycle
MEnd
