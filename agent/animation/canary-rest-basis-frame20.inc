// Issue 36 forensic canary: complete corrected source frame 20 rest-basis pose.
// This is a temporary actual-skin audition; it is never a production move.
// The matching asset is MALE/COHSOURCEDEV_RETARGET_RESTBASIS_FRAME20.

Move COHSOURCEDEV_CUSTOM_CANARY
	Interpolate 5
	Priority 50
	Scale 0
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_RESTBASIS_FRAME20 1 60
		TEnd
	Member "<COHSOURCEDEV_CUSTOM_CANARY>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<COHSOURCEDEV_CUSTOM_CANARY>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>", "<READY>", "<FLYREADY>"
	Requires COHSOURCEDEV_ANIMCANARY
	Flags Cycle
MEnd
