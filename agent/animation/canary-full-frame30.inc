// Issue 36 forensic canary B: FULL_FRAME30.
// Freezes MALE/COHSOURCEDEV_RETARGET_SWING_FULL at runtime frame 30, which the
// numeric decode proved to be the exact static-proof pose (source frame 30;
// runtime frame 0 is the bind reference, so authored frames start at 1).
// Consumed only by the private WebSwingDev player overlay via
// sequencers/cohsourcedev_canary.inc; requires -animcanary 1.

Move COHSOURCEDEV_CUSTOM_CANARY
	Interpolate 5
	Priority 50
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 30 31
		Scale 0
		TEnd
	Member "<COHSOURCEDEV_CUSTOM_CANARY>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<COHSOURCEDEV_CUSTOM_CANARY>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>", "<READY>", "<FLYREADY>"
	Requires COHSOURCEDEV_ANIMCANARY
	Flags Cycle
MEnd
