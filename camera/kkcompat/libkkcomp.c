/*
 * Compatibility hooks required by Smartisan's KitKat libcrypto.
 *
 * Bionic used to export the ARM EABI division-by-zero handler.  Modern
 * bionic no longer does, but the legacy JPEG codec still has an undefined
 * reference to it.  Division helpers pass their return value through this
 * hook when the divisor is zero.
 */
int __aeabi_idiv0(int return_value)
{
    return return_value;
}
