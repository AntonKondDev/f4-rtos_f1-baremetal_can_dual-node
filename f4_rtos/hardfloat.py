from SCons.Script import Import
Import("env")

fpu_flags = [
    "-mcpu=cortex-m4",
    "-mthumb",
    "-mfpu=fpv4-sp-d16",
    "-mfloat-abi=hard",
]

env.Append(ASFLAGS=fpu_flags)
env.Append(CFLAGS=fpu_flags)
env.Append(CXXFLAGS=fpu_flags)
env.Append(CCFLAGS=fpu_flags)  
env.Append(LINKFLAGS=fpu_flags)

env.Append(UNFLAGS=[
    "-mfloat-abi=soft",
    "-mfloat-abi=softfp",
])
