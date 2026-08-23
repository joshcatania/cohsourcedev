// Issue 36 forensic canary A: STATIC_PROOF.
// Freezes MALE/COHSOURCEDEV_RETARGET_POSE_PROOF on its authored proof pose
// (runtime samples 1..N are one constant pose; Scale 0 holds subrange start).
// Consumed only by the private WebSwingDev player overlay via
// sequencers/cohsourcedev_canary.inc; requires -animcanary 1.

Move COHSOURCEDEV_CUSTOM_CANARY
	Interpolate 5
	Priority 50
	Scale 0
	Type Male
		Anim MALE/COHSOURCEDEV_RETARGET_POSE_PROOF 1 60
		TEnd
	Member "<COHSOURCEDEV_CUSTOM_CANARY>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<COHSOURCEDEV_CUSTOM_CANARY>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>", "<READY>", "<FLYREADY>"
	Requires COHSOURCEDEV_ANIMCANARY
	Flags Cycle
MEnd
