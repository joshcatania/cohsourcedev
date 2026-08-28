// Developer-only audition move for authored ANIMX/Blender canaries.
// This include is consumed only by the private WebSwingDev player overlay.

Move COHSOURCEDEV_CUSTOM_CANARY
	Interpolate 5
	Priority 50
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 1 60
		TEnd
	Member "<COHSOURCEDEV_CUSTOM_CANARY>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<COHSOURCEDEV_CUSTOM_CANARY>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>", "<READY>", "<FLYREADY>"
	Requires COHSOURCEDEV_ANIMCANARY
	Flags Cycle
MEnd
