// Issue 36 forensic canary: corrected rest-basis BOTTOM samples 18..22.
// Authored runtime samples 1..5 map to source frames 18..22.
// This is a temporary actual-skin audition; it is never a production move.
// The matching proof asset is MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM.

Move COHSOURCEDEV_CUSTOM_CANARY
	Interpolate 5
	Priority 50
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM 1 5
		TEnd
	Member "<COHSOURCEDEV_CUSTOM_CANARY>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<COHSOURCEDEV_CUSTOM_CANARY>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>", "<READY>", "<FLYREADY>"
	Requires COHSOURCEDEV_ANIMCANARY
	Flags Cycle
MEnd
