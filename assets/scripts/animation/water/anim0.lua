function _Wave01(animScript, deltaTime)
  while true do
    _Wave(animScript, deltaTime, 5, 2.0, 1.0)
    coroutine.yield()
  end
end

local animation = coroutine.create(_Wave01)

function nextWave(animScript, deltaTime)
  if coroutine.status(animation) ~= 'dead' then
    coroutine.resume(animation, animScript, deltaTime)
  end
end
