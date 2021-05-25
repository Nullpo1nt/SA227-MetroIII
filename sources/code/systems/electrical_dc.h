/**
 * Left Essential
 *  -> Bus Tie -> 225 A Fuse -> 325A Fuse (Bat Bus), Left Gen Relay -> Gen
 * Right Essential
 *  -> Bus Tie -> 225 A Fuse -> 325A Fuse (Bat Bus), Right Gen Relay -> Gen
 * Non-Essential
 *  -> Bus Tie -> 150 A CB -> Bat Bus
 *
 * Battery Bus
 *   Left Bat (NiCad) -> Left Bat Relay -> Bat Bus Relay -> Bat Bus
 *   Right Bat (NiCad) -> Right Bat Relay -> Bat Bus Relay -> Bat Bus
 *   GPU -> Bat Bus Relay
 */

// namespace electrical