// Issue 36 Web Swing purpose-built animation set with the Male STRETCH V2
// proof mapped only to the attached/descending Male phases. V1 remains the
// control for Fem, Huge, and the other phases. The installer includes
// this file from the runtime player sequencer after resolving the local player
// source. All three custom logical actions are exported independently for
// Male, Fem, and Huge from the Blender authoring path.
//
// WEBSWING_AIRBORNE intentionally remains the stock airborne control. The
// attached and descending phases share a loop-safe long-body stretch; the
// bottom is a compact athletic tuck; and ascent opens back into a longer,
// outbound silhouette. WEBSWING_RELEASE is intentionally deferred.

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
		Anim MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2 1 30
		TEnd
	Type Huge
		Anim HUGE/COHSOURCEDEV_WEBSWING_STRETCH 1 30
		TEnd
	Type Fem
		Anim FEM/COHSOURCEDEV_WEBSWING_STRETCH 1 30
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
		Anim MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2 1 30
		TEnd
	Type Huge
		Anim HUGE/COHSOURCEDEV_WEBSWING_STRETCH 1 30
		TEnd
	Type Fem
		Anim FEM/COHSOURCEDEV_WEBSWING_STRETCH 1 30
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
		Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 18 22
	TEnd
	Type Huge
		Anim HUGE/COHSOURCEDEV_WEBSWING_TUCK 1 24
	TEnd
	Type Fem
		Anim FEM/COHSOURCEDEV_WEBSWING_TUCK 1 24
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
		Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 30 40
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
