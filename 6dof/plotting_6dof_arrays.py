import matplotlib.pyplot as plt

# Data for each channel
Fx = [-0.0194838, -0.0219193]
Fy = [0.0398085, 0.0398085]
Fz = [-0.0608962, -0.063332]
Mx = [0.0668514, 0.0661003]
My = [0.063332, 0.063332]
Mz = [0.0360428, 0.0360428]



# Time or sample index
time = [1, 2]  # Assuming two samples

# Plot each channel
plt.figure(figsize=(10, 8))

plt.subplot(3, 2, 1)
plt.plot(time, Fx, marker='o', label='Fx')
plt.title('Fx')
plt.grid(True)

plt.subplot(3, 2, 2)
plt.plot(time, Fy, marker='o', label='Fy', color='orange')
plt.title('Fy')
plt.grid(True)

plt.subplot(3, 2, 3)
plt.plot(time, Fz, marker='o', label='Fz', color='green')
plt.title('Fz')
plt.grid(True)

plt.subplot(3, 2, 4)
plt.plot(time, Mx, marker='o', label='Mx', color='red')
plt.title('Mx')
plt.grid(True)

plt.subplot(3, 2, 5)
plt.plot(time, My, marker='o', label='My', color='purple')
plt.title('My')
plt.grid(True)

plt.subplot(3, 2, 6)
plt.plot(time, Mz, marker='o', label='Mz', color='brown')
plt.title('Mz')
plt.grid(True)

plt.tight_layout()
plt.show()